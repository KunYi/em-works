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
//------------------------------------------------------------------------------
//
//  Header: bulverde_usbohci.h
//
//  Defines the USB device controller CPU register layout and definitions.
//
#ifndef __BULVERDE_USBOHCI_H
#define __BULVERDE_USBOHCI_H
#include <xllp_usbohci.h>
//------------------------------------------------------------------------------
//
//  Type: BULVERDE_USBDOHCI_REG    
//
//  Defines the USB OHCI host control register block.
//

typedef XLLP_USBOHCI_T  BULVERDE_USBOHCI_REG;
typedef XLLP_USBOHCI_T *PBULVERDE_USBOHCI_REG;

#ifdef __cplusplus

#include <Cmthread.h>
#include <CRegEdit.h>
//------------------------------------------------------------------------------

#define BULVERDE_REG_DMA_BUFFER_PH_ADDR_VAL_NAME TEXT("DmaMemBase")
#define BULVERDE_REG_DMA_BUFFER_PH_ADDR_VAL_LEN  sizeof(DWORD)
#define BULVERDE_REG_DMA_BUFFER_LENGTH_VAL_NAME  TEXT("DmaMenLen")
#define BULVERDE_REG_DMA_BUFFER_LENGTH_VAL_LEN  sizeof(DWORD)
#define REG_PHYSICAL_PAGE_SIZE TEXT("PhysicalPageSize")
#define REG_PHYSICAL_PAGE_SIZE_LEN sizeof(DWORD)

class SOhcdPdd : public CRegistryEdit, public CMiniThread { 
public:
    SOhcdPdd (LPCTSTR lpActiveRegistry);
    virtual ~SOhcdPdd ();
    virtual BOOL Init();
// Public Function
// Clock Enable
    void TurnOnUSBHostClocks();
    void TurnOffUSBHostClocks();
    void SelectUSBHOSTPowerManagementMode(int Mode,int NumPorts,int *PortMode);
// Power & Pin is provided by BSP
    virtual void SetupUSBHostPWR(int port) = 0;
    virtual void SetupUSBHostPEN(int Port) =0;
// OHCI Configuration.
    virtual BOOL InitializeOHCI();
protected:
    virtual BOOL OHCI_Reset();
    virtual BOOL InitPddInterrupts();
    virtual void DisablePddInterrupts() { 
        m_pDCUSBOHCIReg->uhchie = 0 ; // Mask All PDD interrupt.
    };

private:
    virtual DWORD ThreadRun() { return 1; }   // PDD IST
public:
    virtual void PowerUp();
    virtual void PowerDown();
    virtual DWORD InitiatePowerUp();
    
protected:
    volatile PBULVERDE_CLKMGR_REG m_pDCCLKReg;
    volatile PBULVERDE_USBOHCI_REG m_pDCUSBOHCIReg;
    LPVOID  m_lpvMemoryObject;

    DMA_ADAPTER_OBJECT  m_AdapterObject;
    BOOL                m_bIsBuiltInDma;
    PHYSICAL_ADDRESS    m_DmaPhysicalAddr;
    PVOID               m_pvDmaVirtualAddress;             // DMA buffers as seen by the CPU
    DWORD               m_dwDamBufferSize;
    LPVOID              m_pobMem;

    
    DWORD   m_dwSysIntr;
    HANDLE  m_IsrHandle;

    LPVOID  m_pobOhcd ;

    // Registry Key.
    LPTSTR  m_lpDriverReg;
    HANDLE  m_hParentBusHandle;
};
 
SOhcdPdd * CreateBulverdeOhci(LPCTSTR lpActiveRegistry) ;
#endif

#endif 
