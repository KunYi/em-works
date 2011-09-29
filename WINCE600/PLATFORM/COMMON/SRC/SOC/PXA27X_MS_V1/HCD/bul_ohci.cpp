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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  
    Bul_ohci.cpp
    
Abstract:  
    Bulverde Chip dependant part of the USB Open Host Controller Driver (OHCD).

Notes: 
--*/
#include <windows.h>
#include <ceddk.h>
#include <hcdddsi.h>
#include <bulverde.h>
#include <bulverde_usbohci.h>
// Constant
static const DWORD gcTotalAvailablePhysicalMemory = 65536; // 64K
static const DWORD gcHighPriorityPhysicalMemory = 0x4000; // 16K

LPVOID CreateBulverdeHcdObject(LPVOID lpvHcdPddObject,
        LPVOID lpvMemoryObject, LPCWSTR szRegKey, PUCHAR ioPortBase,
        DWORD dwSysIntr) ;

SOhcdPdd::SOhcdPdd (LPCTSTR lpActiveRegistry)
:   CMiniThread( 0, TRUE )
,   CRegistryEdit(HKEY_LOCAL_MACHINE, lpActiveRegistry )
{
    m_pDCCLKReg = NULL;
    m_pDCUSBOHCIReg = NULL;
    m_lpvMemoryObject = NULL;
    m_pvDmaVirtualAddress = NULL;             // DMA buffers as seen by the CPU
    m_dwDamBufferSize = 0;
    m_dwSysIntr = MAXDWORD;
    m_IsrHandle = NULL;
    m_bIsBuiltInDma = TRUE;
    m_pobMem = NULL;
    m_pobOhcd = NULL;
    m_hParentBusHandle =  CreateBusAccessHandle((LPCWSTR)g_dwContext);
    m_lpDriverReg = NULL;
    if (lpActiveRegistry) {
        DWORD dwLen = _tcslen(lpActiveRegistry);
        m_lpDriverReg = new TCHAR [ dwLen +1 ];
        if (m_lpDriverReg) {
            _tcscpy(m_lpDriverReg,lpActiveRegistry);
            m_lpDriverReg [ dwLen ] =0;
        }
    }
}
SOhcdPdd::~SOhcdPdd ()
{
    m_bTerminated=TRUE;
    ThreadStart();
    // Signal IST.
    ThreadStart();
    DisablePddInterrupts();
    ThreadTerminated(1000);
    //
    if (m_IsrHandle) {
        FreeIntChainHandler(m_IsrHandle);
    }
    
    if ( m_pobOhcd )
        HcdMdd_DestroyHcdObject( m_pobOhcd );
    if (m_pobMem)
        HcdMdd_DestroyMemoryObject(m_pobMem);
    
    if (m_pvDmaVirtualAddress) {        
        if (m_bIsBuiltInDma) {
            MmUnmapIoSpace(m_pvDmaVirtualAddress,0);
        }
        else {
            HalFreeCommonBuffer(&m_AdapterObject, m_dwDamBufferSize, m_DmaPhysicalAddr, m_pvDmaVirtualAddress, FALSE);
        }
    }
    if (m_lpDriverReg)
        delete m_lpDriverReg;
    if (m_pDCCLKReg)
        MmUnmapIoSpace((PVOID)m_pDCCLKReg,0);
    if (m_pDCUSBOHCIReg)
        MmUnmapIoSpace((PVOID)m_pDCUSBOHCIReg,0);
    if (m_pvDmaVirtualAddress)
        MmUnmapIoSpace(m_pvDmaVirtualAddress,0);
    if (m_hParentBusHandle)
        CloseBusAccessHandle(m_hParentBusHandle);

}
BOOL SOhcdPdd::Init()
{
    {
        PHYSICAL_ADDRESS ioPhysicalBase = { BULVERDE_BASE_REG_PA_CLKMGR, 0};
        m_pDCCLKReg = (PBULVERDE_CLKMGR_REG)MmMapIoSpace(ioPhysicalBase, sizeof(BULVERDE_CLKMGR_REG),FALSE);
    }
    {
        PHYSICAL_ADDRESS ioPhysicalBase = { BULVERDE_BASE_REG_PA_USBH, 0};
        m_pDCUSBOHCIReg = (PBULVERDE_USBOHCI_REG)MmMapIoSpace(ioPhysicalBase, sizeof(BULVERDE_USBOHCI_REG),FALSE);
    }
    if (m_pDCCLKReg && m_pDCUSBOHCIReg && IsKeyOpened()) {
        TurnOnUSBHostClocks();
        // Port 1
        SetupUSBHostPWR(1);
        SetupUSBHostPEN(1);
        // Port 2
        SetupUSBHostPWR(2);
        SetupUSBHostPEN(2);
        return (InitPddInterrupts() && OHCI_Reset() && InitializeOHCI());
    }
    return FALSE;
}
BOOL SOhcdPdd::InitializeOHCI()
{
    PUCHAR ioPortBase = NULL;
    DWORD dwAddrLen;
    DWORD dwIOSpace;
    BOOL InstallIsr = FALSE;
    BOOL fResult = FALSE;
    LPVOID pobMem = NULL;
    LPVOID pobOhcd = NULL;
    DWORD PhysAddr;
    HKEY hKey=NULL;

    DDKWINDOWINFO dwi;
    DDKISRINFO dii;

    // Setup Memory Windows For OHCI MDD code.
    dwi.cbSize=sizeof(dwi);
    PhysAddr = dwi.memWindows[0].dwBase = BULVERDE_BASE_REG_PA_USBH;
    dwAddrLen= dwi.memWindows[0].dwLen = 0x1000;
    dwi.dwInterfaceType = Internal;
    dwi.dwBusNumber = 0;
    dwi.dwNumMemWindows =1;
    dwIOSpace = 0;
        
    dii.cbSize=sizeof(dii);
    if (GetIsrInfo(&dii)  != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("InitializeOHCI:DDKReg_GetWindowInfo or  DDKReg_GetWindowInfo failed\r\n")));
        return FALSE;
    }
    // Overwrite the item we know.
    dii.dwIrq = IRQ_USBOHCI;
    m_dwSysIntr = dii.dwSysintr;
    DEBUGMSG(ZONE_INIT,(TEXT("OHCD: Read config from registry: Base Address: 0x%X, Length: 0x%X, I/O Port: %s, SysIntr: 0x%X, Interface Type: %u, Bus Number: %u\r\n"),
                    PhysAddr, dwAddrLen, dwIOSpace ? L"YES" : L"NO", dii.dwSysintr, dwi.dwInterfaceType, dwi.dwBusNumber));
    
    ioPortBase = (PBYTE) PhysAddr;
    
    if (dii.szIsrDll[0] != 0 && dii.szIsrHandler[0]!=0 && dii.dwIrq<0xff && dii.dwIrq>0 ) {
        // Install ISR handler
        m_IsrHandle = LoadIntChainHandler(dii.szIsrDll, dii.szIsrHandler, (BYTE)dii.dwIrq);

        if (!m_IsrHandle) {
            DEBUGMSG(ZONE_ERROR, (L"OHCD: Couldn't install ISR handler\r\n"));
        } else {
            GIISR_INFO Info;
            PHYSICAL_ADDRESS PortAddress = {PhysAddr, 0};
            
            DEBUGMSG(ZONE_INIT, (L"OHCD: Installed ISR handler, Dll = '%s', Handler = '%s', Irq = %d\r\n",
                dii.szIsrDll, dii.szIsrHandler, dii.dwIrq));
            
            if (!BusTransBusAddrToStatic(m_hParentBusHandle,(INTERFACE_TYPE)dwi.dwInterfaceType, dwi.dwBusNumber, PortAddress, dwAddrLen, &dwIOSpace, (PVOID *)&PhysAddr)) {
                DEBUGMSG(ZONE_ERROR, (L"OHCD: Failed TransBusAddrToStatic\r\n"));
                return FALSE;
            }
        
            // Set up ISR handler
            Info.SysIntr = dii.dwSysintr;
            Info.CheckPort = TRUE;
            Info.PortIsIO = (dwIOSpace) ? TRUE : FALSE;
            Info.UseMaskReg = TRUE;
            Info.PortAddr = PhysAddr + 0x0C;
            Info.PortSize = sizeof(DWORD);
            Info.MaskAddr = PhysAddr + 0x10;
            
            if (!KernelLibIoControl(m_IsrHandle, IOCTL_GIISR_INFO, &Info, sizeof(Info), NULL, 0, NULL)) {
                DEBUGMSG(ZONE_ERROR, (L"OHCD: KernelLibIoControl call failed.\r\n"));
            }
        }
    }
    DWORD dwDmaBase = 0;
    DWORD dwHPPhysicalMemSize = 0;
    if (!GetRegValue(BULVERDE_REG_DMA_BUFFER_PH_ADDR_VAL_NAME, (PBYTE)&dwDmaBase,BULVERDE_REG_DMA_BUFFER_PH_ADDR_VAL_LEN)) {
        DEBUGMSG(ZONE_INIT, (TEXT("SOhcdPdd::Init - Cann't get \"%s\" registry value.\r\n"),BULVERDE_REG_DMA_BUFFER_PH_ADDR_VAL_NAME));
        dwDmaBase = 0;
    }
    if (!GetRegValue(BULVERDE_REG_DMA_BUFFER_LENGTH_VAL_NAME, (PBYTE)&m_dwDamBufferSize,BULVERDE_REG_DMA_BUFFER_LENGTH_VAL_LEN)) {
        DEBUGMSG(ZONE_INIT, (TEXT("SOhcdPdd::Init - Cann't get \"%s\" registry value.\r\n"),BULVERDE_REG_DMA_BUFFER_LENGTH_VAL_NAME));
        m_dwDamBufferSize = 0;
    }
    if (dwDmaBase && m_dwDamBufferSize) {
        m_DmaPhysicalAddr.LowPart = dwDmaBase;
        m_DmaPhysicalAddr.HighPart = 0;
        m_pvDmaVirtualAddress = MmMapIoSpace(m_DmaPhysicalAddr,m_dwDamBufferSize, FALSE);
        dwHPPhysicalMemSize = m_dwDamBufferSize / 4  ; 
        m_bIsBuiltInDma = TRUE;
    }
    else { // we locate DMA buffer from public pool
        // The PDD can supply a buffer of contiguous physical memory here, or can let the 
        // MDD try to allocate the memory from system RAM.  We will use the HalAllocateCommonBuffer()
        // API to allocate the memory and bus controller physical addresses and pass this information
        // into the MDD.
        if (GetRegValue(REG_PHYSICAL_PAGE_SIZE, (PBYTE)&m_dwDamBufferSize, REG_PHYSICAL_PAGE_SIZE_LEN)) {
            dwHPPhysicalMemSize = m_dwDamBufferSize/4;
            m_dwDamBufferSize = (m_dwDamBufferSize+ PAGE_SIZE -1) & ~(PAGE_SIZE -1);
            // Align with page size.        
            dwHPPhysicalMemSize = ((dwHPPhysicalMemSize +  PAGE_SIZE -1) & ~(PAGE_SIZE -1));
        }
        else 
            m_dwDamBufferSize=0;
        
        if (m_dwDamBufferSize<gcTotalAvailablePhysicalMemory) { // Setup Minimun requirement.
            m_dwDamBufferSize = gcTotalAvailablePhysicalMemory;
            dwHPPhysicalMemSize = gcHighPriorityPhysicalMemory;
        }

        m_AdapterObject.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);
        m_AdapterObject.InterfaceType = Internal;
        m_AdapterObject.BusNumber = 0 ;
        if ((m_pvDmaVirtualAddress = HalAllocateCommonBuffer(&m_AdapterObject, m_dwDamBufferSize, &m_DmaPhysicalAddr, FALSE)) == NULL) {
            DEBUGMSG(ZONE_INIT, (TEXT("SOhcdPdd::InitializeOHCI() - HalAllocateCommonBuffer return FALSE!!!\r\n.\r\n")));
            return FALSE;
        }
        m_bIsBuiltInDma = FALSE;
    }
    if (m_pvDmaVirtualAddress == NULL || 
            !(m_pobMem = HcdMdd_CreateMemoryObject(m_dwDamBufferSize, dwHPPhysicalMemSize, (PUCHAR) m_pvDmaVirtualAddress, (PUCHAR) m_DmaPhysicalAddr.LowPart))) {
        DEBUGMSG(ZONE_INIT, (TEXT("SOhcdPdd::InitializeOHCI() - Cann't CreateMemoryObject!!!\r\n.\r\n")));
        return FALSE;
    }

    if (!( m_pobOhcd  = CreateBulverdeHcdObject(this, m_pobMem, m_lpDriverReg, (PUCHAR)m_pDCUSBOHCIReg, m_dwSysIntr))) {
        DEBUGMSG(ZONE_INIT, (TEXT("SOhcdPdd::InitializeOHCI() - Cann't CreateHcdObject!!!\r\n.\r\n")));
        return FALSE;
    }

    return TRUE;
}
BOOL SOhcdPdd::InitPddInterrupts()
{
    DEBUGMSG(ZONE_INIT, (TEXT("SOhcdPdd::InitPddInterrupts() - Initial UHCHIE = 0x%x\r\n"),m_pDCUSBOHCIReg->uhchie));
    m_pDCUSBOHCIReg->uhchie = 0 ; // Mask All PDD interrupt.
    return TRUE;
}

BOOL SOhcdPdd::OHCI_Reset()
{
   DEBUGMSG(ZONE_INIT,(TEXT("OHCI_Reset: Resetting Bulverde OHCI.\r\n")));

   // Do the reset for the Bulverde part.
   // Two levels of reset need to be initiated:
   //	The OHCI core needs to be reset via the FHR bit,
   //	then the OHCI system bus interface needs to be reset via the FSBIR bit.

    // reset the OHC core and all OHC blocks driven by the 12 MHz clock, eg. write fifo, etc.
    m_pDCUSBOHCIReg->uhchr |=  XLLP_USBOHCI_UHCHR_FHR;
    Sleep(1);
    m_pDCUSBOHCIReg->uhchr &= ~XLLP_USBOHCI_UHCHR_FHR;

    // reset the OHC system bus interface, eg. SBI, DMA blocks, fifos, etc.
    m_pDCUSBOHCIReg->uhchr |=  XLLP_USBOHCI_UHCHR_FSBIR;
    while( m_pDCUSBOHCIReg->uhchr & XLLP_USBOHCI_UHCHR_FSBIR );	// auto clears in 3 system bus clocks

    DEBUGMSG(m_pDCUSBOHCIReg,(TEXT("OHCI_Reset: done.\r\n")));
    return TRUE;
}

// TurnOnUSBHostClocks:
//      This routine will make sure that the USB Host OHCI block in the
//      Bulverde core is getting clocks. If it is not getting clocks,
//      then some accesses to it may stall, especially if one needs to
//      wait for some OHCI register bits to change.
void
SOhcdPdd::TurnOnUSBHostClocks()
{
    // The clock enable bit for the USB Host OHCI block in Bulverde
    // is bit number 20.
    DEBUGMSG(ZONE_INIT,(TEXT("TurnOnUSBHostClocks: Initial Values: cccr: %08x cken: %08x oscc: %08x ccsr: %08x\n\r"), m_pDCCLKReg->cccr, m_pDCCLKReg->cken, m_pDCCLKReg->oscc, m_pDCCLKReg->ccsr));
    m_pDCCLKReg->cken |= XLLP_CLKEN_USBHOST;
    DEBUGMSG(ZONE_INIT,(TEXT("TurnOnUSBHostClocks: Final   Values: cccr: %08x cken: %08x oscc: %08x ccsr: %08x\n\r"), m_pDCCLKReg->cccr, m_pDCCLKReg->cken, m_pDCCLKReg->oscc, m_pDCCLKReg->ccsr));
}


// TurnOffUSBHostClocks:
//      This routine will make sure that the USB Host OHCI block in the
//      Bulverde core is not getting clocks. If it is not getting clocks,
//      then there will be power savings.
void
SOhcdPdd::TurnOffUSBHostClocks()
{
    // The clock enable bit for the USB Host OHCI block in Bulverde
    // is bit number 20.
    DEBUGMSG(ZONE_INIT,(TEXT("TurnOfUSBHostClocks: Initial Values: cccr: %08x cken: %08x oscc: %08x ccsr: %08x\n\r"), m_pDCCLKReg->cccr, m_pDCCLKReg->cken, m_pDCCLKReg->oscc, m_pDCCLKReg->ccsr));
    m_pDCCLKReg->cken &= ~XLLP_CLKEN_USBHOST;
    DEBUGMSG(ZONE_INIT,(TEXT("TurnOfUSBHostClocks: Final   Values: cccr: %08x cken: %08x oscc: %08x ccsr: %08x\n\r"), m_pDCCLKReg->cccr, m_pDCCLKReg->cken, m_pDCCLKReg->oscc, m_pDCCLKReg->ccsr));
}


void
SOhcdPdd::SelectUSBHOSTPowerManagementMode(
    int	Mode,
    int	NumPorts,
    int	*PortMode
    )
{
    switch(Mode) {
      case XLLP_USBOHCI_PPM_NPS:
        // set NO Power Switching mode
        m_pDCUSBOHCIReg->uhcrhda |= XLLP_USBOHCI_UHCRHDA_NPS;			
        break;

      case XLLP_USBOHCI_PPM_GLOBAL:
        // make sure the NO Power Switching mode bit is OFF so Power Switching can occur
        // make sure the PSM bit is CLEAR, which allows all ports to be controlled with 
        // the GLOBAL set and clear power commands
        m_pDCUSBOHCIReg->uhcrhda &= ~(XLLP_USBOHCI_UHCRHDA_NPS|XLLP_USBOHCI_UHCRHDA_PSM_PERPORT);
        break;

      case XLLP_USBOHCI_PPM_PERPORT:
        // make sure the NO Power Switching mode bit is OFF so Power Switching can occur
        // make sure the PSM bit is SET, which allows all ports to be controlled with 
        // the PER PORT set and clear power commands
        m_pDCUSBOHCIReg->uhcrhda &= ~XLLP_USBOHCI_UHCRHDA_NPS;
        m_pDCUSBOHCIReg->uhcrhda |=  XLLP_USBOHCI_UHCRHDA_PSM_PERPORT;

        // set the power management mode for each individual port to Per Port.
        {
            int p;

            for( p = 0; p < NumPorts; p++ ) {
                m_pDCUSBOHCIReg->uhcrhdb |= (unsigned int)( 1u << (p+17) );	// port 1 begins at bit 17
            }
        }

        break;

      case XLLP_USBOHCI_PPM_MIXED:
        // make sure the NO Power Switching mode bit is OFF so Power Switching can occur
        // make sure the PSM bit is SET, which allows all ports to be controlled with 
        // the PER PORT set and clear power commands
        m_pDCUSBOHCIReg->uhcrhda &= ~XLLP_USBOHCI_UHCRHDA_NPS;
        m_pDCUSBOHCIReg->uhcrhda |=  XLLP_USBOHCI_UHCRHDA_PSM_PERPORT;

        // set the power management mode for each individual port to Per Port.
        // if the value in the PortMode array is non-zero, set Per Port mode for the port.
        // if the value in the PortMode array is zero, set Global mode for the port
        {
        int		p;

        for( p = 0; p < NumPorts; p++ ) {
            if( PortMode[p] ) {
                m_pDCUSBOHCIReg->uhcrhdb |= (unsigned int)( 1u << (p+17) );	// port 1 begins at bit 17
            }
            else   {
                m_pDCUSBOHCIReg->uhcrhdb &= ~(unsigned int)( 1u << (p+17) );	// port 1 begins at bit 17
            }

            }
        }

        break;
    }
}




// Manage WinCE suspend/resume events

DWORD SOhcdPdd::InitiatePowerUp()
{
    DEBUGMSG(ZONE_INIT,(TEXT("SOhcdPdd::InitiatePowerUp: m_pDCUSBOHCIReg: %08x.\r\n"), m_pDCUSBOHCIReg));


    TurnOnUSBHostClocks();	// make sure the ohci block is running (eg. getting clocked)
    // Port 1
    SetupUSBHostPWR(1);		// this sets up Pwr 1 notification using gpio 88 as input in alternate function 1 mode
    SetupUSBHostPEN(1);		// this sets up Pwr 1 enable using gpio 89 as output in alternate function 2 mode
    // Port 2
    SetupUSBHostPWR(2);		// this sets up Pwr 2 notification using gpio 88 as input in alternate function 1 mode
    SetupUSBHostPEN(2);		// this sets up Pwr 2 enable using gpio 89 as output in alternate function 2 mode
    
    //TurnOnUSBHostPorts();	// probably only do this after the rest of the ohci is set up.
    //TestUSBHostPEN(0);
    SelectUSBHOSTPowerManagementMode( XLLP_USBOHCI_PPM_NPS, 0, 0 );
    
    OHCI_Reset();
    return 0;
}
void SOhcdPdd::PowerUp()
{
    HcdMdd_PowerUp(m_pobOhcd);
}

void SOhcdPdd::PowerDown()
{

    // let the MDD do its processing (including putting the HC into reset)
    HcdMdd_PowerDown(m_pobOhcd);

    // disable the USB port as described in section 6.1.4.4 of the SA-1111 companion
    // chip documentation:
    // (1) Reset HC (done by MDD)
    // (2) wait 10 us
    // (3) clear global power enable bit
    // (4) set the standby enable bit
    // (5) stop the usb clock
    //usWait(10);                     // must not block or do operations illegal in interrupt context
    m_pDCUSBOHCIReg->uhcrhda &= ~ ((1 << 8) | (1 << 9));     // set global power switch mode
    m_pDCUSBOHCIReg->uhcrhs |= 0x0001;                   // clear global power
};


/* HcdPdd_DllMain
 * 
 *  DLL Entry point.
 *
 * Return Value:
 */
extern "C" BOOL HcdPdd_DllMain(HANDLE /*hinstDLL*/, DWORD /*dwReason*/, LPVOID /*lpvReserved*/)
{
    return TRUE;
}
// This gets called by the MDD's IST when it detects a power resume.
extern "C" void HcdPdd_InitiatePowerUp (DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;
    if (pPddObject)
        pPddObject->InitiatePowerUp();
    
    return;
}


/* HcdPdd_Init
 *
 *   PDD Entry point - called at system init to detect and configure UHCI card.
 *
 * Return Value:
 *   Return pointer to PDD specific data structure, or NULL if error.
 */
extern "C" DWORD 
HcdPdd_Init(
    DWORD dwContext)  // IN - Pointer to context value. For device.exe, this is a string 
                      //      indicating our active registry key.
{
    SOhcdPdd *  pPddObject = CreateBulverdeOhci((LPCTSTR)dwContext);

    if (pPddObject && pPddObject->Init()) {
        DEBUGMSG(ZONE_INIT, (TEXT("HcdPdd_Init: Checking SW18 - controls OHCI loading.\r\n")));
        return (DWORD) pPddObject ;
    }
    if (pPddObject)
        delete pPddObject;
    return (DWORD)NULL;
}

/* HcdPdd_CheckConfigPower
 *
 *    Check power required by specific device configuration and return whether it
 *    can be supported on this platform.  For CEPC, this is trivial, just limit to
 *    the 500mA requirement of USB.  For battery powered devices, this could be 
 *    more sophisticated, taking into account current battery status or other info.
 *
 * Return Value:
 *    Return TRUE if configuration can be supported, FALSE if not.
 */
extern "C" BOOL HcdPdd_CheckConfigPower(
    UCHAR bPort,         // IN - Port number
    DWORD dwCfgPower,    // IN - Power required by configuration
    DWORD dwTotalPower)  // IN - Total power currently in use on port
{
    return ((dwCfgPower + dwTotalPower) > 500) ? FALSE : TRUE;
}

extern "C" void HcdPdd_PowerUp(DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;
    DEBUGMSG(ZONE_INIT, (TEXT("HcdPdd_PowerUp: enter.\n\r")));
    if (pPddObject)
        pPddObject->PowerUp();
    DEBUGMSG(ZONE_INIT, (TEXT("HcdPdd_PowerUp: Need to add Bulverde support.\n\r")));
    return;
}

extern "C" void HcdPdd_PowerDown(DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;
    DEBUGMSG(ZONE_INIT, (TEXT("HcdPdd_PowerDown: enter.\n\r")));
    if (pPddObject)
        pPddObject->PowerDown();

    DEBUGMSG(ZONE_INIT, (TEXT("HcdPdd_PowerDown: Need to add Bulverde support.\n\r")));
    return;
}


extern "C" BOOL HcdPdd_Deinit(DWORD hDeviceContext)
{
    SOhcdPdd * pPddObject = (SOhcdPdd *)hDeviceContext;
    if (pPddObject)
        delete pPddObject;
    return TRUE;
}


extern "C" DWORD HcdPdd_Open(DWORD /*hDeviceContext*/, DWORD /*AccessCode*/,
        DWORD /*ShareMode*/)
{

    return 1; // we can be opened, but only once!
}


extern "C" BOOL HcdPdd_Close(DWORD /*hOpenContext*/)
{
    return TRUE;
}


extern "C" DWORD HcdPdd_Read(DWORD /*hOpenContext*/, LPVOID /*pBuffer*/, DWORD /*Count*/)
{
    return (DWORD)-1; // an error occured
}


extern "C" DWORD HcdPdd_Write(DWORD /*hOpenContext*/, LPCVOID /*pSourceBytes*/,
        DWORD /*NumberOfBytes*/)
{
    return (DWORD)-1;
}


extern "C" DWORD HcdPdd_Seek(DWORD /*hOpenContext*/, LONG /*Amount*/, DWORD /*Type*/)
{
    return (DWORD)-1;
}


extern "C" BOOL HcdPdd_IOControl(DWORD /*hOpenContext*/, DWORD /*dwCode*/, PBYTE /*pBufIn*/,
        DWORD /*dwLenIn*/, PBYTE /*pBufOut*/, DWORD /*dwLenOut*/, PDWORD /*pdwActualOut*/)
{
    return FALSE;
}

#include <COhcd.hpp>

class CBulverdeOhcd : public COhcd {
public:
    CBulverdeOhcd ( IN LPVOID pvOhcdPddObject,
                     IN CPhysMem * pCPhysMem,
                     IN LPCWSTR szDriverRegistryKey,
                     IN REGISTER portBase,
                     IN DWORD dwSysIntr)
    :   COhcd(pvOhcdPddObject,pCPhysMem,szDriverRegistryKey,portBase,dwSysIntr)
    { ; }

    //
    // Root Hub Queries
    //
    virtual UCHAR GetNumberOfDownStreamPort()  {
        return (COhcd::GetNumberOfDownStreamPort()+1) ;
    }

    //
    // Load "HcdCapability" value from the registry key
    //
    virtual void LoadHcdCapability( IN LPCWSTR szDriverRegistryKey) {
        LONG  regError;
        DWORD dwDataSize;
        HKEY  hKey = NULL;
        DWORD dwHcdCapability=0;

        // Open the driver registry key
        regError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szDriverRegistryKey,0,KEY_ALL_ACCESS,&hKey);
        if (regError != ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_INIT,(TEXT("CBulverdeOhcd:LoadHcdCapability:: Failed opening HKLM\\%s \r\n"), szDriverRegistryKey));
            hKey = NULL;
        }
            
        // Get "HcdCapability"
        if (hKey)
        {
            dwDataSize = sizeof(dwHcdCapability);
            regError = RegQueryValueEx(hKey,HCD_CAPABILITY_VALNAME,NULL,NULL,(LPBYTE)&dwHcdCapability,&dwDataSize);
            if (regError == ERROR_SUCCESS)
            {
                this->SetCapability(dwHcdCapability);
            }
            else
            {
                DEBUGMSG(ZONE_INIT,(TEXT("CBulverdeOhcd:LoadHcdCapability:: Failed opening HKLM\\%s\\%s \r\n"), szDriverRegistryKey,HCD_CAPABILITY_VALNAME));
            }
        }

        RegCloseKey (hKey);
    }
};

LPVOID CreateBulverdeHcdObject (LPVOID lpvHcdPddObject,
        LPVOID lpvMemoryObject, LPCWSTR szRegKey, PUCHAR ioPortBase,
        DWORD dwSysIntr)
{
    CHcd * pobHcd = new CBulverdeOhcd (lpvHcdPddObject, (CPhysMem *)lpvMemoryObject,szRegKey,ioPortBase,dwSysIntr) ;
    if ( pobHcd != NULL ) {

        ((CBulverdeOhcd*)pobHcd)->LoadHcdCapability(szRegKey);

        if ( !pobHcd->DeviceInitialize( )) {
            delete pobHcd;
            pobHcd = NULL;
        }
    }

    return pobHcd;
}

