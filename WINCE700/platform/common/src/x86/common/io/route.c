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
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//  
// -----------------------------------------------------------------------------
#include <windows.h>
#include <oal.h>
#include <nkintr.h>
#include <x86boot.h>
#include <x86kitl.h>
#include <oal_intr.h>
#include <PCIReg.h>
#include "pci.h"

//----------------------------------------------------------------------------------
// PCI BIOS (Routing Table Support)
static const DWORD BIOS_START = 0xe0000;
static const DWORD BIOS_LENGTH = 0x20000;

#pragma pack(1)

typedef struct {
    BYTE PCIBusNumber;
    BYTE PCIDeviceNumber;// In upper 5bits
    
    BYTE INTA_LinkValue;
    unsigned short INTA_IrqBitMap;
    
    BYTE INTB_LinkValue;
    unsigned short INTB_IrqBitMap;
    
    BYTE INTC_LinkValue;
    unsigned short INTC_IrqBitMap;
    
    BYTE INTD_LinkValue;
    unsigned short INTD_IrqBitMap;

    BYTE SlotNumber;
    BYTE Reserved;
} RoutingOptionTable;

typedef struct {
    DWORD   Signature;
    WORD    Version;
    WORD    Table_Size;
    BYTE    Bus;
    BYTE    DeviceFunc;
    WORD    ExclusiveIrqs;
    DWORD   CompatibleRouter;
    DWORD   MiniportData;
    BYTE    Reseved[11];
    BYTE    Checksum;
} PCI_ROUTING_TABLE;

typedef struct 
{
    ULONG   magic;
    ULONG   phys_bsd_entry;
    UCHAR   vers,prg_lens,crc;
} BIOS32;

typedef struct {
    DWORD RegEax;
    DWORD RegEbx;
    DWORD RegEcx;
    DWORD RegEdx;
    DWORD RegEsi;
    DWORD RegEdi;
} Reg32;

enum { MAX_DEVICE = 0x20 };

typedef struct {
    unsigned short BufferSize;
    BYTE * pDataBuffer;
    DWORD DS;
    RoutingOptionTable routingOptionTable[MAX_DEVICE];
} IRQRountingOptionsBuffer;

typedef struct _irqLink {
    BYTE linkValue;
    BYTE bus;
    BYTE device;
    BYTE func;
    struct _irqLink* pNext;
} irqLink, *pIrqLink;

#pragma pack() 

// this function is implemented in assembly as _CallBios32
extern BOOL CallBios32(Reg32 * pReg,void* pCallAddr);

enum 
{
    PCIFN_PCI_BIOS_PRESENT       = 0xB101,
    PCIFN_FIND_PCI_DEVICE        = 0xB102,
    PCIFN_FIND_PCI_CLASS_CODE    = 0xB103,
    PCIFN_GENERATE_SPECIAL_CYCLE = 0xB106,
    PCIFN_READ_CONFIG_BYTE       = 0xB108,
    PCIFN_READ_CONFIG_WORD       = 0xB109,
    PCIFN_READ_CONFIG_DWORD      = 0xB10A,
    PCIFN_WRITE_CONFIG_BYTE      = 0xB10B,
    PCIFN_WRITE_CONFIG_WORD      = 0xB10C,
    PCIFN_WRITE_CONFIG_DWORD     = 0xB10D,
    PCIFN_GET_IRQ_ROUTING_OPTIONS = 0xB10E,
    PCIFN_SET_PCI_IRQ            = 0xB10F
};


enum { NUM_IRQS = 0x10 };
static pIrqLink irqToLinkValue[NUM_IRQS];

// limit the pool to 1 page in size
enum { IRQ_LINK_POOL_SIZE = ((VM_PAGE_SIZE / (sizeof(irqLink))) - NUM_IRQS) };
// Simple allocation pool for the irqToLinkValue table
static irqLink irqLinkPool[IRQ_LINK_POOL_SIZE];


//------------------------------------------------------------------------------
//
//  Function:  SearchPciBios
//
//  
//
static BOOL SearchPciBios(
                          __in const BYTE* const pBiosAddr,
                          __in ULONG * pphOffset
                          )
{
    DWORD       p=0;
    UCHAR       flag=0;
    UCHAR       crc;
    int         i;

    OALMSG(OAL_PCI,(L"SearchPciBios start\r\n"));
    while(pphOffset && flag==0 && p<BIOS_LENGTH)
    {
        const BIOS32* const x=(const BIOS32*)(pBiosAddr+p);
        if (x->magic=='_23_')
        {
            for(i=0, crc=0; i<(x->prg_lens*16); i++)
            crc+=*(pBiosAddr+p+i);
            if(crc==0)
            {
                const BIOS32 * const master_bios32=x;
                flag=1;
                *pphOffset=master_bios32->phys_bsd_entry-BIOS_START;
                OALMSG(OAL_PCI,(L"CE Ethernet Bootloader found 32Bit BIOS Entry master_bios32=%x bios32_call_offset=%x for CE/PC \r\n",
                    master_bios32,*pphOffset));
                return TRUE;
            }
        }
        p+=0x10;
    }
    OALMSG(OAL_ERROR,(L"SearchPciBios end fails\r\n"));
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  AllocFreeIrqLink
//
//  allocates an unused link from the allocation pool.  Note that there is no way to dealloc as it is
//  expected that this table will only need to be set up once.
//
static pIrqLink AllocFreeIrqLink()
{
    static unsigned int irqLinkPoolCounter = 0;
    if(irqLinkPoolCounter >= (IRQ_LINK_POOL_SIZE - 1)) 
    {
        // We've run out of memory in our pool
        OALMSG(OAL_ERROR, (L"irqLinkPoolCounter (%d) exceeded allocation pool size!\r\n", irqLinkPoolCounter));
        return 0;
    }
    else 
    {
        // Allocate the next free block
        irqLinkPoolCounter++;
        return &(irqLinkPool[irqLinkPoolCounter]);
    }
}

//------------------------------------------------------------------------------
//
//  Function:  AddIrqLink
//
//  allocates an irqLink at inIrqLink and fills the value with the link number
//
static BOOL AddIrqLink(
                       __inout pIrqLink* inIrqLink, 
                       DWORD inIrq, 
                       BYTE inLinkNumber, 
                       BYTE inBus, 
                       BYTE inDevice,
                       BYTE inFunc
                       )
{
    *inIrqLink = AllocFreeIrqLink();
    if(!(*inIrqLink)) 
    {
        OALMSG(OAL_ERROR, (L"Failed allocation for IrqToLink, link %x will not be associated with Irq %d\r\n", inLinkNumber, inIrq));
    } 
    else 
    {
        // Allocation successful, update the entry with the new value
        (*inIrqLink)->linkValue = inLinkNumber;
        (*inIrqLink)->bus = inBus;
        (*inIrqLink)->device = inDevice;        
        (*inIrqLink)->func = inFunc;   
        OALMSG(OAL_PCI,(L"AddIrqLink: LinkNumber=%x,bus=%d,device=%d,func=%d associated with irq=%d\r\n",inLinkNumber,inBus,inDevice,inFunc,inIrq));
        return TRUE;
    }
    return FALSE;
}

static PVOID pBiosAddr=NULL;
static IRQRountingOptionsBuffer irqRoutingOptionBuffer;
static WORD wBestPCIIrq=0;

extern DWORD GetDS();

//------------------------------------------------------------------------------
//
//  Function:  DumpRoutingOption
//
//  
//
static void DumpRoutingOption(
                              __in const IRQRountingOptionsBuffer * const pBuffer,
                              WORD wExClusive
                              )
{
    OALMSG(OAL_PCI,(L"DumpRoutingOption with PCI Exclusive Irq Bit (wExClusive)  =%x \r\n",wExClusive));
    if (pBuffer) 
    {
        const RoutingOptionTable * pRoute = (pBuffer->routingOptionTable);
        DWORD dwCurPos=0;
        OALMSG(OAL_PCI,(L"DumpRoutingOption BufferSize = %d @ address %x \r\n", pBuffer->BufferSize,pBuffer->pDataBuffer));
        while (pRoute && dwCurPos + sizeof(RoutingOptionTable)<=pBuffer->BufferSize) 
        {
            OALMSG(OAL_PCI,(L"DumpRouting for Bus=%d ,Device=%d SlotNumber=%d\r\n",
                pRoute->PCIBusNumber,(pRoute->PCIDeviceNumber)>>3,pRoute->SlotNumber));
            OALMSG(OAL_PCI,(L"     INTA_LinkValue=%x,INTA_IrqBitMap=%x\r\n",pRoute->INTA_LinkValue,pRoute->INTA_IrqBitMap));
            OALMSG(OAL_PCI,(L"     INTB_LinkValue=%x,INTB_IrqBitMap=%x\r\n",pRoute->INTB_LinkValue,pRoute->INTB_IrqBitMap));
            OALMSG(OAL_PCI,(L"     INTC_LinkValue=%x,INTC_IrqBitMap=%x\r\n",pRoute->INTC_LinkValue,pRoute->INTC_IrqBitMap));
            OALMSG(OAL_PCI,(L"     INTD_LinkValue=%x,INTC_IrqBitMap=%x\r\n",pRoute->INTD_LinkValue,pRoute->INTD_IrqBitMap));
            dwCurPos +=sizeof(RoutingOptionTable);
            pRoute ++;
        }
    }
}

static PBYTE pBiosVirtAddr=NULL;

//------------------------------------------------------------------------------
//
//  Function:  SearchPciRouting
//
//  
//
static BOOL SearchPciRouting(
                             __out PCI_ROUTING_TABLE ** pphAddr
                             )
{
    DWORD       p=0;
    PCI_ROUTING_TABLE      *x;
    UCHAR       flag=0;
    UCHAR       crc;
    int         i;

    OALMSG(OAL_PCI,(L"SearchPciRouting\r\n"));
    while(pBiosVirtAddr && pphAddr && flag==0 && (ULONG)p<BIOS_LENGTH)
    {
        x=(PCI_ROUTING_TABLE *)(pBiosVirtAddr+p);
        if (x->Signature=='RIP$')
        {
            for(i=0, crc=0; i<x->Table_Size; i++)
                crc+=*(pBiosVirtAddr+p+i);
            if(crc==0)
            {
                flag=1;
                *pphAddr=x;
                OALMSG(OAL_PCI,(L"SearchPciRouting found entry =%x  CE/PC \r\n",*pphAddr));
                return TRUE;
            }
            else
                OALMSG(OAL_ERROR,(L"SearchPciRouting Entry Checksum Error @%x\r\n",x));
        }
        p+=0x10;
    }
    OALMSG(OAL_ERROR,(L"SearchPciRouting end fails\r\n"));
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  BiosMapIrq
//
//  
//
static BOOL BiosMapIrq(
                       __in const DEVICE_LOCATION* pDevLoc,
                       BYTE bIrq
                       )
{
    Reg32 mReg32;
    BOOL  bRet;
    if (pDevLoc && pBiosAddr) 
    {
        mReg32.RegEax=PCIFN_SET_PCI_IRQ;
        mReg32.RegEcx=((DWORD)bIrq)*0x100 + (pDevLoc->Pin & 0xff ) + 9;
        mReg32.RegEbx=
            (pDevLoc->BusNumber & 0xff) * 0x100 +  // Bus Number
            ((pDevLoc->LogicalLoc >> 5) & 0xF8) +  // Device Number
            ((pDevLoc->LogicalLoc ) & 7 );  //Function Number.
        OALMSG(OAL_PCI,(L"BiosMapIrq(EAX=%x,EBX=%x,ECX=%x)\r\n",mReg32.RegEax,mReg32.RegEbx,mReg32.RegEcx));
        bRet=CallBios32(&mReg32,pBiosAddr);
        OALMSG(OAL_PCI,(L"BiosMapIrq return %d and EAX=%x\r\n",bRet,mReg32.RegEax));
        return bRet;
    }
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  ScanConfiguredIrq
//
//  
//
static void ScanConfiguredIrq(
                              __in const IRQRountingOptionsBuffer *pBuffer,
                              WORD wExClusive
                              )
{
    DWORD Irq;
    DWORD Pin;

    OALMSG(OAL_PCI,(L"ScanConfiguredIrq with PCI Exclusive Irq Bit (wExClusive)  =%x \r\n",wExClusive));
    memset(irqToLinkValue, 0, sizeof(irqToLinkValue));
    memset(irqLinkPool, 0, sizeof(irqLinkPool));
    wBestPCIIrq=wExClusive;
    
    if (pBuffer) 
    {
        const RoutingOptionTable * pRoute = (const RoutingOptionTable * )(pBuffer->pDataBuffer);
        DWORD dwCurPos=0;
        OALMSG(OAL_PCI,(L"ScanConfigureIrq: BufferSize = %d @ address %x \r\n", pBuffer->BufferSize,pBuffer->pDataBuffer));
        while (pRoute && dwCurPos + sizeof(RoutingOptionTable)<=pBuffer->BufferSize) 
        {
            BOOL isMultiFunc = FALSE;
            DWORD dwFunc;
            OALMSG(OAL_PCI,(L"ScanConfigureIrq: for Bus=%d ,Device=%d SlotNumber=%d\r\n",
                pRoute->PCIBusNumber,(pRoute->PCIDeviceNumber)>>3,pRoute->SlotNumber));
            OALMSG(OAL_PCI,(L"     INTA_LinkValue=%x,INTA_IrqBitMap=%x\r\n",pRoute->INTA_LinkValue,pRoute->INTA_IrqBitMap));
            OALMSG(OAL_PCI,(L"     INTB_LinkValue=%x,INTB_IrqBitMap=%x\r\n",pRoute->INTB_LinkValue,pRoute->INTB_IrqBitMap));
            OALMSG(OAL_PCI,(L"     INTC_LinkValue=%x,INTC_IrqBitMap=%x\r\n",pRoute->INTC_LinkValue,pRoute->INTC_IrqBitMap));
            OALMSG(OAL_PCI,(L"     INTD_LinkValue=%x,INTD_IrqBitMap=%x\r\n",pRoute->INTD_LinkValue,pRoute->INTD_IrqBitMap));
            for (dwFunc=0;dwFunc<8;dwFunc++) 
            {
                PCI_COMMON_CONFIG pciConfig;
                DWORD dwLength;
                pciConfig.VendorID = 0xFFFF;
                pciConfig.HeaderType = 0;

                dwLength=PCIReadBusData(pRoute->PCIBusNumber,(pRoute->PCIDeviceNumber)>>3,dwFunc,
                          &pciConfig,0,sizeof(pciConfig) - sizeof(pciConfig.DeviceSpecific));
                if (dwFunc==0)
                    isMultiFunc = (pciConfig.HeaderType & PCI_MULTIFUNCTION)?TRUE:FALSE;

                if (dwLength != (sizeof(pciConfig) - sizeof(pciConfig.DeviceSpecific)) ||
                        (pciConfig.DeviceID == PCI_INVALID_DEVICEID) || (pciConfig.VendorID == PCI_INVALID_VENDORID) || (pciConfig.VendorID == 0)) 
                {
                    if (dwFunc == 0) 
                    {
                        // If not a multi-function device, continue to next device
                        if (!isMultiFunc) 
                            break;
                    }
                    continue;
                }

                Irq=(DWORD)-1;
                Pin=0;
                switch( pciConfig.HeaderType & ~PCI_MULTIFUNCTION) 
                {
                    case PCI_DEVICE_TYPE: // Devices
                        Irq = pciConfig.u.type0.InterruptLine;
                        Pin = pciConfig.u.type0.InterruptPin;
                        break;
                    case PCI_BRIDGE_TYPE: // PCI-PCI bridge
                        Irq = pciConfig.u.type1.InterruptLine;
                        Pin = pciConfig.u.type1.InterruptPin;
                        break;

                    case PCI_CARDBUS_TYPE: // PCI-Cardbus bridge
                        Irq = pciConfig.u.type2.InterruptLine;
                        Pin = pciConfig.u.type2.InterruptPin;
                        break;
                }

                if (Irq<=0xf) 
                {
                    BYTE bLinkNumber=0;
                    switch (Pin) 
                    {
                        case 1:
                            bLinkNumber=pRoute->INTA_LinkValue;
                            break;
                        case 2:
                            bLinkNumber=pRoute->INTB_LinkValue;
                            break;
                        case 3:
                            bLinkNumber=pRoute->INTC_LinkValue;
                            break;
                        case 4:
                            bLinkNumber=pRoute->INTD_LinkValue;
                            break;
                    }
                    if (bLinkNumber!=0) 
                    {
                        if (irqToLinkValue[Irq]) 
                        {
                            // Already some links associated with this irq

                            // Search the list for a duplicate entry
                            pIrqLink currentIrqToLink = irqToLinkValue[Irq];
                            BOOL matchFound = FALSE;
                            do {
                                if(currentIrqToLink->linkValue == bLinkNumber &&
                                   currentIrqToLink->bus == pRoute->PCIBusNumber &&
                                   currentIrqToLink->device == ((pRoute->PCIDeviceNumber)>>3) &&
                                   currentIrqToLink->func == dwFunc)
                                {
                                    matchFound = TRUE;                                        
                                }
                                if(currentIrqToLink->pNext) 
                                {
                                    currentIrqToLink = currentIrqToLink->pNext;
                                } 
                                else 
                                {
                                    break;
                                }
                            } while (!matchFound);

                            if(!matchFound) 
                            {
                                // No association for this link with this irq, so add a new entry to pNext
                                AddIrqLink(&(currentIrqToLink->pNext), Irq, bLinkNumber, pRoute->PCIBusNumber, ((pRoute->PCIDeviceNumber)>>3), (BYTE)dwFunc);
                            }                                
                        } 
                        else 
                        {
                            // No links associated with this irq, add a new entry to the list
                            AddIrqLink(&(irqToLinkValue[Irq]), Irq, bLinkNumber, pRoute->PCIBusNumber, ((pRoute->PCIDeviceNumber)>>3), (BYTE)dwFunc);
                        }
                    }
                }
                if (!isMultiFunc) 
                {
                    // Not multi-function card.
                    break;
                }
            }
            dwCurPos +=sizeof(RoutingOptionTable);
            pRoute ++;
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function:  GetRoutingOption
//
//  
//
static BOOL GetRoutingOption(
                             __inout IRQRountingOptionsBuffer * pBuffer,
                             __in void* phPciBiosAddr
                             )
{
    Reg32 mReg32;
    PCI_ROUTING_TABLE * pPCIRoutingTable=NULL;
    OALMSG(OAL_PCI,(L"+GetRoutingOption\r\n"));
    if (pBiosVirtAddr!=NULL && SearchPciRouting(&pPCIRoutingTable) && pPCIRoutingTable) 
    {
        DWORD dwCurPos=sizeof(PCI_ROUTING_TABLE);
        const RoutingOptionTable * curTable=(const RoutingOptionTable *)(pPCIRoutingTable+1);
        DWORD dwIndex=0;
        pBuffer->BufferSize=0;
        pBuffer->pDataBuffer = (PBYTE)(pBuffer->routingOptionTable);
        OALMSG(OAL_PCI,(L"GetRoutingOption, found ROM version for Routing table.\r\n"));
        while (dwCurPos + sizeof(RoutingOptionTable)<=pPCIRoutingTable->Table_Size && dwIndex < MAX_DEVICE ) 
        {
            memcpy(pBuffer->routingOptionTable+dwIndex,curTable,sizeof(RoutingOptionTable));
            dwIndex++;
            curTable++;
            dwCurPos +=sizeof(RoutingOptionTable);
            pBuffer->BufferSize +=sizeof(RoutingOptionTable);
        }
        OALMSG(OAL_PCI,(L"GetRoutingOption return SUCCESS .AH=%x \r\n",pPCIRoutingTable->ExclusiveIrqs));
        ScanConfiguredIrq(pBuffer,(WORD)pPCIRoutingTable->ExclusiveIrqs);
        return TRUE;
    }
    else if (pBuffer && phPciBiosAddr) 
    {
        pBuffer->BufferSize = sizeof(pBuffer->routingOptionTable);
        pBuffer->pDataBuffer = (PBYTE)(pBuffer->routingOptionTable);
        pBuffer->DS=GetDS();
        OALMSG(OAL_PCI,(L"GetRoutingOption with buffer Size %d bytes buffer DS%x:addr =%x\r\n ",pBuffer->BufferSize,pBuffer->DS,pBuffer->pDataBuffer));
        mReg32.RegEax=PCIFN_GET_IRQ_ROUTING_OPTIONS;
        mReg32.RegEbx=0;
        mReg32.RegEdi=(DWORD)pBuffer;
        if (CallBios32(&mReg32, phPciBiosAddr)) 
        { 
            // Success
            OALMSG(OAL_PCI,(L"GetRoutingOption return SUCCESS .AH=%x\r\n ",(mReg32.RegEax & 0xff00)>>8));
            ScanConfiguredIrq(pBuffer,(WORD)mReg32.RegEbx);
            return TRUE;
        }
        else 
        {
            pBuffer->pDataBuffer = NULL;// Routing does not exist.
            OALMSG(OAL_ERROR,(L"GetRoutingOption return FAILS error code AH=%x \r\n",(mReg32.RegEax & 0xff00)>>8));
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  GetPciRoutingIrqTable
//
//  
//
static void GetPciRoutingIrqTable()
{
    ULONG phBiosOffset=0;

    // Initial the talbe.
    memset(&irqRoutingOptionBuffer,0,sizeof(irqRoutingOptionBuffer));
    pBiosAddr=NULL;

    // Maping BIOS address 
    if (!pBiosVirtAddr)
        // 64k From E0000-FFFFF
        pBiosVirtAddr=(PBYTE)NKCreateStaticMapping((DWORD)BIOS_START>>8,BIOS_LENGTH);

    OALMSG(OAL_PCI,(L"PCIBIOS:: BIOS Address static map to addr=%x\r\n",pBiosVirtAddr));

    if (!pBiosVirtAddr)
        return;
    
    OALMSG(OAL_PCI,(L"GetPicRoutingIrqTable: Start\r\n"));
    
    if (SearchPciBios(pBiosVirtAddr,&phBiosOffset) && phBiosOffset!=0) 
    {
        // Find out the address off $PCI service.
        Reg32 mReg32;
        mReg32.RegEax='ICP$';
        mReg32.RegEbx=0;
        
        CallBios32(&mReg32,pBiosVirtAddr+phBiosOffset);
        
        OALMSG(OAL_PCI,(L"Return from First BIOS EAX=%x EBX=%x,ECX=%x EDX=%x\r\n",
                mReg32.RegEax,mReg32.RegEbx,mReg32.RegEcx,mReg32.RegEdx));

        if ((mReg32.RegEax & 0xff)==0) 
        { 
            // Success to load and PCI calls
            phBiosOffset=mReg32.RegEbx+mReg32.RegEdx-BIOS_START;

            OALMSG(OAL_PCI,(L"32 PCI BIOS offset located.addr=%x\r\n",phBiosOffset));

            mReg32.RegEax=PCIFN_PCI_BIOS_PRESENT;

            if (CallBios32(&mReg32,pBiosVirtAddr+phBiosOffset) &&  (mReg32.RegEbx & 0xffff)>=0x210 ) 
            { 
                OALMSG(OAL_PCI,(L"32 PCI BIOS Present EDX=%x,EAX=%x EBX=%x,ECX=%x\r\n",
                    mReg32.RegEdx,mReg32.RegEax,mReg32.RegEbx,mReg32.RegEcx));

                pBiosAddr=pBiosVirtAddr+phBiosOffset;
                GetRoutingOption(&irqRoutingOptionBuffer,pBiosAddr);
                return;
            }
        }
    }
    else if (GetRoutingOption(&irqRoutingOptionBuffer, NULL))
    {
        // We can still find the IRQ routing table even if there is no BIOS32 support.
        return;
    }
    
    OALMSG(OAL_ERROR,(L"GetPicRoutingIrqTable: Failed\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  OALIntrRequestIrqs
//
//  
//
BOOL OALIntrRequestIrqs(
                        PDEVICE_LOCATION pDevLoc, 
                        __inout UINT32 *pCount, 
                        __out UINT32 *pIrq
                        )
{
    int Bus = (pDevLoc->LogicalLoc >> 16) & 0xFF;
    int Device = (pDevLoc->LogicalLoc >> 8) & 0xFF;
    int Func = pDevLoc->LogicalLoc & 0xFF;

    // Figure out the Link number in routing table
    const RoutingOptionTable * pRoute =(const RoutingOptionTable * )irqRoutingOptionBuffer.pDataBuffer;

    OALMSG(OAL_PCI,(L"OALIntrRequestIrqs:(Bus=%d,Device=%d,Pin=%d)\r\n",Bus,Device,pDevLoc->Pin));

    // This shouldn't happen
    if (*pCount < 1) return FALSE;
    if (pRoute) 
    {
        // serch table.
        DWORD dwCurPos=0;
        while (dwCurPos + sizeof(RoutingOptionTable)<=irqRoutingOptionBuffer.BufferSize) 
        {            
            OALMSG(OAL_PCI,(L"OALIntrRequestIrqs: for Bus=%d ,Device=%d SlotNumber=%d\r\n",
                pRoute->PCIBusNumber,(pRoute->PCIDeviceNumber)>>3,pRoute->SlotNumber));
            OALMSG(OAL_PCI,(L"     INTA_LinkValue=%x,INTA_IrqBitMap=%x\r\n",pRoute->INTA_LinkValue,pRoute->INTA_IrqBitMap));
            OALMSG(OAL_PCI,(L"     INTB_LinkValue=%x,INTB_IrqBitMap=%x\r\n",pRoute->INTB_LinkValue,pRoute->INTB_IrqBitMap));
            OALMSG(OAL_PCI,(L"     INTC_LinkValue=%x,INTC_IrqBitMap=%x\r\n",pRoute->INTC_LinkValue,pRoute->INTC_IrqBitMap));
            OALMSG(OAL_PCI,(L"     INTD_LinkValue=%x,INTC_IrqBitMap=%x\r\n",pRoute->INTD_LinkValue,pRoute->INTD_IrqBitMap));
            if (pRoute->PCIBusNumber== Bus && ((pRoute->PCIDeviceNumber)>>3)==Device) 
            { 
                // found
                BYTE bLinkNumber=0;
                WORD wIntPossibleBit=0;
                switch (pDevLoc->Pin) 
                {
                case 1:
                    bLinkNumber=pRoute->INTA_LinkValue;
                    wIntPossibleBit=pRoute->INTA_IrqBitMap;
                    break;
                case 2:
                    bLinkNumber=pRoute->INTB_LinkValue;
                    wIntPossibleBit=pRoute->INTB_IrqBitMap;
                    break;
                case 3:
                    bLinkNumber=pRoute->INTC_LinkValue;
                    wIntPossibleBit=pRoute->INTC_IrqBitMap;
                    break;
                case 4:
                    bLinkNumber=pRoute->INTD_LinkValue;
                    wIntPossibleBit=pRoute->INTD_IrqBitMap;
                    break;
                }
                if (bLinkNumber!=0) 
                {
                    DWORD dwIndex;
                    BYTE  bIrq=(BYTE)-1;
                    WORD wIntrBit;
                    int iIndex;
                    for (dwIndex = 0; dwIndex < NUM_IRQS; dwIndex++) 
                    {
                        // Traverse the list for each irq and search for a matching
                        // bus, device, and linkNumber.
                        // If we find one this IRQ has already been mapped
                        pIrqLink currentIrqToLink = irqToLinkValue[dwIndex];
                        while (currentIrqToLink) 
                        {
                            OALMSG(OAL_PCI,(L"Traverse IrqLinks - IRQ %d maps to link %x bus %d device %d func %d\r\n", dwIndex,
                                           currentIrqToLink->linkValue, currentIrqToLink->bus, currentIrqToLink->device, currentIrqToLink->func));
                            if(currentIrqToLink->linkValue == bLinkNumber &&
                               currentIrqToLink->bus == pRoute->PCIBusNumber &&
                               currentIrqToLink->device == ((pRoute->PCIDeviceNumber)>>3) &&
                               currentIrqToLink->func == Func) 
                            {
                                if (pIrq) *pIrq=dwIndex;
                                if (pCount) *pCount = 1;
                                OALMSG(OAL_PCI,(L"-OALIntrRequestIrqs: Found full IRQ match returning existing IRQ=%d\r\n",dwIndex));
                                return TRUE;
                            }
                            currentIrqToLink = currentIrqToLink->pNext;
                        }
                    }
                    for (dwIndex = 0; dwIndex < NUM_IRQS; dwIndex++) 
                    {
                        // Traverse the list for each irq and search for a matching
                        // linkNumber only.  This is our second-best metric for finding a match.
                        pIrqLink currentIrqToLink = irqToLinkValue[dwIndex];
                        while (currentIrqToLink) 
                        {
                            OALMSG(OAL_PCI,(L"Traverse IrqLinks - IRQ %d maps to link %x bus %d device %d\r\n", dwIndex,
                                           currentIrqToLink->linkValue, currentIrqToLink->bus, currentIrqToLink->device));
                            if(currentIrqToLink->linkValue == bLinkNumber) 
                            {
                                if (pIrq) *pIrq=dwIndex;
                                if (pCount) *pCount = 1;
                                OALMSG(OAL_PCI,(L"-OALIntrRequestIrqs: Found linkNumber IRQ match returning existing IRQ=%d\r\n",dwIndex));
                                return TRUE;
                            }
                            currentIrqToLink = currentIrqToLink->pNext;
                        }
                    }
                    // We didn't match up with any IRQs, search for a useful Interrupt
                    wIntrBit=0x8000;
                    for (iIndex=(NUM_IRQS-1);iIndex>=0;iIndex--) 
                    {
                        if ((wBestPCIIrq & wIntrBit)!=0 && irqToLinkValue[iIndex]==0 ) 
                        { 
                            // Best interurpt is can be mapped but not mapped yet
                            bIrq=(BYTE)iIndex;
                            OALMSG(OAL_PCI,(L"OALIntrRequestIrqs: Try mapping New IRQ(%d) to this device\r\n",bIrq));
                            if (bIrq<=(NUM_IRQS-1) && BiosMapIrq(pDevLoc,bIrq)) 
                            { 
                                // Mapped.
                                // Scan to the end and add an association for the irq and linkvalue
                                pIrqLink currentIrqToLink = irqToLinkValue[iIndex];
                                while(currentIrqToLink) 
                                {
                                    currentIrqToLink = currentIrqToLink->pNext;
                                }
                                AddIrqLink(&(irqToLinkValue[iIndex]), bIrq, bLinkNumber, pRoute->PCIBusNumber, ((pRoute->PCIDeviceNumber)>>3), Func);
                                if (pIrq) *pIrq=(DWORD)bIrq;
                                if (pCount) *pCount = 1;
                                OALMSG(OAL_PCI,(L"-OALIntrRequestIrqs: Mapped New IRQ return IRQ=%d\r\n",bIrq));
                                return TRUE;
                            }
                        }
                        wIntrBit>>=1;
                    }
                    wIntrBit=0x8000;
                    for (iIndex=(NUM_IRQS-1);iIndex>=0;iIndex--) 
                    {
                        if ((wIntPossibleBit & wIntrBit)!=0 && irqToLinkValue[iIndex]==0 ) 
                        { 
                            // Best interurpt is can be mapped but not mapped yet
                            bIrq=(BYTE)iIndex;
                            OALMSG(OAL_PCI,(L"OALIntrRequestIrqs: Try mapping New IRQ(%d) to this device\r\n",bIrq));
                            if (bIrq<=(NUM_IRQS-1) && BiosMapIrq(pDevLoc,bIrq)) 
                            { 
                                // Mapped.
                                // Scan to the end and add an association for the irq and linkvalue
                                pIrqLink currentIrqToLink = irqToLinkValue[iIndex];
                                while(currentIrqToLink) 
                                {
                                    currentIrqToLink = currentIrqToLink->pNext;
                                }
                                AddIrqLink(&(irqToLinkValue[iIndex]), bIrq, bLinkNumber, pRoute->PCIBusNumber, ((pRoute->PCIDeviceNumber)>>3), Func);
                                if (pIrq) *pIrq=(DWORD)bIrq;
                                if (pCount) *pCount = 1;
                                OALMSG(OAL_PCI,(L"-OALIntrRequestIrqs: Mapped New IRQ return IRQ=%d\r\n",bIrq));
                                return TRUE;
                            }
                        }
                        wIntrBit>>=1;
                    }
                }
                break;// False
                
            }
            dwCurPos +=sizeof(RoutingOptionTable);
            pRoute ++;
        }
    }
    return FALSE;
}

BOOL PCIInitConfigMechanism (UCHAR ucConfigMechanism);

//------------------------------------------------------------------------------
//
//  Function:  PCIInitBusInfo
//
//  
//
void PCIInitBusInfo()
{
    if (PCIInitConfigMechanism (g_pX86Info->ucPCIConfigType & 0x03)) 
    {
        GetPciRoutingIrqTable ();
    } 
    else 
    {
        OALMSG(OAL_LOG_INFO, (TEXT("No PCI bus\r\n")));
    }
}

BOOL 
PCIConfigureIrq (
    IN ULONG BusNumber,
    IN ULONG DeviceNumber,
    IN ULONG FunctionNumber,
    IN ULONG PinNumber,
    IN ULONG IrqToUse,
    IN BOOL  ForceIfMultipleDevs
    )
{
    Reg32 mReg32;
    DWORD dwIndex,dwCurPos;
    pIrqLink pLink;
    BYTE newLinkValue;
    BOOL bRet = FALSE;
    const RoutingOptionTable * pRoute =(const RoutingOptionTable * )irqRoutingOptionBuffer.pDataBuffer;

    OALMSG(OAL_PCI,(L"+PCIConfigureIrq\r\n"));
    do {
        if ((!pRoute) || (!pBiosAddr))
        {
            OALMSG(OAL_ERROR,(L"PCIConfigureIrq: No routing table available.\r\n"));
            break;
        }

        if (IrqToUse >= NUM_IRQS)
        {
            OALMSG(OAL_ERROR,(L"PCIConfigureIrq: Invalid IRQ specified (%d)\r\n", IrqToUse));
            break;
        }

        /* look for the IRQ currently in use by this device */
        dwIndex = 0;
        do {
            pLink = irqToLinkValue[dwIndex];
            while (pLink)
            {
                if ((pLink->bus == BusNumber) &&
                    (pLink->device == DeviceNumber) &&
                    (pLink->func == FunctionNumber))
                    break;
                pLink = pLink->pNext;
            }
            if (pLink)
                break;
        } while (++dwIndex < NUM_IRQS);

        if (pLink)
        {
            OALMSG(OAL_PCI,(L"PCIConfigureIrq: %d/%d/%d currently set to use IRQ %d (LV %d)\r\n",BusNumber,DeviceNumber,FunctionNumber,dwIndex, pLink->linkValue));
            if (dwIndex==IrqToUse)
            {
                OALMSG(OAL_PCI,(L"PCIConfigureIrq: Configuring for current IRQ.  No-Op.\r\n"));
                bRet = TRUE;
                break;
            }
            if ((irqToLinkValue[dwIndex]!=pLink) || (pLink->pNext))
            {
                if (!ForceIfMultipleDevs)
                {
                    OALMSG(OAL_ERROR,(L"***ERROR: PCIConfigureIrq: attempt to set %d/%d/%d to IRQ %d but other devices use the same hardware trace and would also change!\r\n",
                        BusNumber, DeviceNumber, FunctionNumber, IrqToUse));
                    break;
                }
                else
                {
                    OALMSG(OAL_WARN,(L"!!!WARNING: PCIConfigureIrq: setting %d/%d/%d to IRQ %d but other devices use the same hardware trace and will also change!\r\n",
                        BusNumber, DeviceNumber, FunctionNumber, IrqToUse));
                }
            }
        }
        else
        {
            OALMSG(OAL_PCI,(L"PCIConfigureIrq: %d/%d/%d does not have a current interrupt assignment we know about\r\n",BusNumber,DeviceNumber,FunctionNumber));
        }

        mReg32.RegEax=PCIFN_SET_PCI_IRQ;
        mReg32.RegEcx=((DWORD)IrqToUse)*0x100 + (PinNumber & 0xff ) + 9;
        mReg32.RegEbx=((BusNumber & 0xff) * 0x100) |
                      (DeviceNumber << 3) |
                      (FunctionNumber & 7);
        bRet = CallBios32(&mReg32,pBiosAddr);

        if (bRet && ((mReg32.RegEax & 0xFF00)==0))
        {
            /* switched over device to use the new IRQ.  Update the irqToLinkValue chains */

            /* there may not have been devices on the new link value before */
            /* scan the (instantly updated by the BIOS) routing table for the device's new link value */
            dwCurPos = 0;
            newLinkValue = 0;
            while ((dwCurPos + sizeof(RoutingOptionTable)) <= irqRoutingOptionBuffer.BufferSize)
            {
                if ((pRoute->PCIBusNumber==BusNumber) &&
                    ((pRoute->PCIDeviceNumber>>3)==DeviceNumber))
                {
                    switch (PinNumber)
                    {
                        case 1:
                            newLinkValue = pRoute->INTA_LinkValue;
                            break;
                        case 2:
                            newLinkValue = pRoute->INTB_LinkValue;
                            break;
                        case 3:
                            newLinkValue = pRoute->INTC_LinkValue;
                            break;
                        case 4:
                            newLinkValue = pRoute->INTD_LinkValue;
                            break;
                    }
                    break;
                }
                dwCurPos +=sizeof(RoutingOptionTable);
                pRoute ++;
            }

            if (!newLinkValue)
            {
                OALMSG(OAL_ERROR,(L"***ERROR: PCIConfigureIrq: Routing updated but could not find device link number in updated bios table.\r\n"));
                bRet = FALSE;
                break;
            }

            OALMSG(OAL_PCI,(L"PCIConfigureIrq: Device(s) on IRQ %d switched to IRQ %d.\r\n", dwIndex, IrqToUse));

            if (!pLink)
            {
                /* device previously did not have an IRQ */
                AddIrqLink(&(irqToLinkValue[IrqToUse]), IrqToUse, newLinkValue, (BYTE)BusNumber, (BYTE)DeviceNumber, (BYTE)FunctionNumber);
            }
            else
            {
                /* device previously had an IRQ (and may have had friends sharing it) */
                pLink = irqToLinkValue[IrqToUse];
                if (pLink)
                {
                    /* other stuff was already on IrqToUse IRQ */
                    while (pLink->pNext)
                    {
                        pLink = pLink->pNext;
                    }
                    pLink->pNext = irqToLinkValue[dwIndex];
                }
                else
                    irqToLinkValue[IrqToUse] = irqToLinkValue[dwIndex];
                pLink = irqToLinkValue[dwIndex];
                irqToLinkValue[dwIndex] = NULL;
                while (pLink)
                {
                    pLink->linkValue = newLinkValue;
                    pLink = pLink->pNext;
                }
            }

            OALMSG(OAL_PCI,(L"PCIConfigureIrq: Device(s) on IRQ %d (LV%d) are now:\r\n", IrqToUse, newLinkValue));
            pLink = irqToLinkValue[IrqToUse];
            while (pLink)
            {
                OALMSG(OAL_PCI,(L"%d/%d/%d LV %d\r\n", pLink->bus, pLink->device, pLink->func, pLink->linkValue));
                pLink = pLink->pNext;
            }
        }
        else
        {
            OALMSG(OAL_ERROR,(L"***ERROR: PCIConfigureIrq: Failed to set PCI IRQ %d. Result code 0x%02X\r\n", IrqToUse, (mReg32.RegEax & 0xFF00)>>8));
            bRet = FALSE;
        }

    } while (0);

    OALMSG(OAL_PCI,(L"-PCIConfigureIrq(returns %s)\r\n", bRet?L"TRUE":L"FALSE"));
    return bRet;
}
