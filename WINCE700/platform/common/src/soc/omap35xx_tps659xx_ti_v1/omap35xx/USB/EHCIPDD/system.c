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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:
    system.c

Abstract:
    Device dependant part of the USB Universal Host Controller Driver (EHCD).

Notes:
--*/

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
//
//  File:  system.c
//
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ddkreg.h>
#include <devload.h>
#include <giisr.h>
#include <hcdddsi.h>
#include <cebuscfg.h>
#include <omap35xx_config.h>
#include <omap35xx_irq.h>
#include <omap35xx_prcm.h>
#include <omap35xx_base_regs.h>
#include <omap35xx_usbhost.h>
#include <ceddkex.h>
#include <constants.h>
#include <initguid.h>
#include <gpio.h>
#include <omap35xx.h>
#include <bus.h>

#ifdef DEBUG
#define ZONE_POWER              DEBUGZONE(9)
#define ZONE_INFO               DEBUGZONE(10)
#define ZONE_PDD                DEBUGZONE(15)

//#undef ZONE_INFO
//#define ZONE_INFO 1
//#undef ZONE_ERROR
//#define ZONE_ERROR 1

// set to non-zero value to cause register dump after configuration
#define DUMP_REGS       0

#endif

#define EHCI_SUSPEND_RESUME_CLOCKS      0

#define EHCI_DMA_ALLOC                  0

#define REG_PHYSICAL_PAGE_SIZE      TEXT("PhysicalPageSize")

#define MODEM_SW_RST                    BIT0
#define MODEM_SW_RSTPWRON           BIT1

#define EHCI_PRCM_TIMEOUT               1000

#define NUM_PORTS                   3

enum
{
    USB_PORT_NOT_USED = 0,
    USB_PORT_PHY_MODE,
    USB_PORT_TLL_MODE
};


typedef struct _SEHCDPdd
{
    DWORD memBase;
    DWORD memLen;
    DWORD irq;
    DWORD pwrdownSuspendBus;
    LPVOID lpvMemoryObject;
    LPVOID lpvEHCDMddObject;
    PVOID pvVirtualAddress;                 // DMA buffers as seen by the CPU
    DWORD dwPhysicalMemSize;
    PHYSICAL_ADDRESS LogicalAddress;        // DMA buffers as seen by the DMA controller and bus interfaces
    DMA_ADAPTER_OBJECT AdapterObject;
    TCHAR szDriverRegKey[MAX_PATH];
    PUCHAR ioPortBase;
    DWORD dwSysIntr;
    CRITICAL_SECTION csPdd;                 // serializes access to the PDD object
    HANDLE          IsrHandle;
    HANDLE hParentBusHandle;
    DWORD port1Mode;
    DWORD port2Mode;
    DWORD port3Mode;
    DWORD Port1PwrGpio;
    DWORD Port2PwrGpio;
    DWORD Port3PwrGpio;
    DWORD Port1PwrLevel;
    DWORD Port2PwrLevel;
    DWORD Port3PwrLevel;
    HANDLE hGpio;
    HANDLE hRootBus;

    UINT nDVFSOrder;
} SEHCDPdd, *P_SEHCDPdd;

//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"MemBase", PARAM_DWORD, TRUE, offset(SEHCDPdd, memBase),
        fieldsize(SEHCDPdd, memBase), NULL
    }, { 
        L"MemLen", PARAM_DWORD, TRUE, offset(SEHCDPdd, memLen),
        fieldsize(SEHCDPdd, memLen), NULL
    }, { 
        L"Irq", PARAM_DWORD, TRUE, offset(SEHCDPdd, irq),
        fieldsize(SEHCDPdd, irq), NULL
    }, { 
        L"PwrdownSuspendBus", PARAM_DWORD, FALSE, offset(SEHCDPdd, pwrdownSuspendBus),
        fieldsize(SEHCDPdd, pwrdownSuspendBus), (void *)1                 // ; Either 0 (false) or 1 (true)
    }, { 
        L"Port1Mode", PARAM_DWORD, FALSE, offset(SEHCDPdd, port1Mode),
        fieldsize(SEHCDPdd, port1Mode), (void *)USB_PORT_PHY_MODE                 // ;  0 (not used), 1 (PHY), 2 (TLL)
    }, { 
        L"Port2Mode", PARAM_DWORD, FALSE, offset(SEHCDPdd, port2Mode),
        fieldsize(SEHCDPdd, port2Mode), (void *)USB_PORT_PHY_MODE                 // ;  0 (not used), 1 (PHY), 2 (TLL)
    }, { 
        L"Port3Mode", PARAM_DWORD, FALSE, offset(SEHCDPdd, port3Mode),
        fieldsize(SEHCDPdd, port3Mode), (void *)USB_PORT_NOT_USED                 // ;  0 (not used), 1 (PHY), 2 (TLL)
    }, { 
        L"Port1PwrGpio", PARAM_DWORD, FALSE, offset(SEHCDPdd, Port1PwrGpio),
        fieldsize(SEHCDPdd, Port1PwrGpio), (void *) -1                            // ;  -1 (not used), CPU GPIO used to control VBUS
    }, { 
        L"Port2PwrGpio", PARAM_DWORD, FALSE, offset(SEHCDPdd, Port2PwrGpio),
        fieldsize(SEHCDPdd, Port2PwrGpio), (void *) -1                            // ;  -1 (not used), CPU GPIO used to control VBUS
    }, { 
        L"Port3PwrGpio", PARAM_DWORD, FALSE, offset(SEHCDPdd, Port3PwrGpio),
        fieldsize(SEHCDPdd, Port3PwrGpio), (void *) -1                            // ;  -1 (not used), CPU GPIO used to control VBUS
    }, { 
        L"Port1PwrLevel", PARAM_DWORD, FALSE, offset(SEHCDPdd, Port1PwrLevel),
        fieldsize(SEHCDPdd, Port1PwrLevel), (void *) 0                            // ;  PortnPwrGpio level to enable VBUS, 0 active low, 1 active high
    }, { 
        L"Port2PwrLevel", PARAM_DWORD, FALSE, offset(SEHCDPdd, Port2PwrLevel),
        fieldsize(SEHCDPdd, Port2PwrLevel), (void *) 0                            // ;  PortnPwrGpio level to enable VBUS, 0 active low, 1 active high
    }, { 
        L"Port3PwrLevel", PARAM_DWORD, FALSE, offset(SEHCDPdd, Port3PwrLevel),
        fieldsize(SEHCDPdd, Port3PwrLevel), (void *) 0                            // ;  PortnPwrGpio level to enable VBUS, 0 active low, 1 active high
    }, { 
        L"DVFSOrder", PARAM_DWORD, FALSE, offset(SEHCDPdd, nDVFSOrder),
        fieldsize(SEHCDPdd, nDVFSOrder), (void *)150
    } 
};

extern BOOL PHY_AssertReset(SEHCDPdd * pPddObject, BOOL bCalledFromPowerUp);
extern BOOL PHY_DeassertReset(SEHCDPdd * pPddObject);
extern BOOL TLLDevice_AssertReset(SEHCDPdd * pPddObject);
extern BOOL TLLDevice_DeassertReset(SEHCDPdd * pPddObject);
extern void dumpRegs(void);

#define UnusedParameter(x)  x = x

// Amount of memory to use for HCD buffer
#define gcTotalAvailablePhysicalMemory (0x20000) // 128K
#define gcHighPriorityPhysicalMemory (gcTotalAvailablePhysicalMemory/4)   // 1/4 as high priority

#define START_WRITE_ULPI_DATA(port, reg, data)    (0x80000000 | ((port) << 24) | (2 << 22) | ((reg) << 16) | data)
#define START_READ_ULPI_DATA(port, reg)    (0x80000000 | ((port) << 24)  | (3 << 22)| ((reg) << 16))

void DelayMilliSeconds(DWORD dwMilliSeconds, BOOL bInPowerHandler)
{
    if (bInPowerHandler)
    {
        DWORD StartingTickCount;

        StartingTickCount = GetTickCount();
        while (GetTickCount() - StartingTickCount < dwMilliSeconds + 1)
        {
        }       
    }
    else
    {
        Sleep(dwMilliSeconds);
    }
}

void EhciWriteReg(DWORD EhciRegBase, DWORD Offset, DWORD Data)
{
    volatile DWORD * pEhciReg = (volatile DWORD *)(EhciRegBase + Offset);
    
    *pEhciReg = Data;
}

void UlpiWriteReg(DWORD EhciRegBase, DWORD Port, DWORD Register, BYTE Data)
{
    volatile DWORD * pEhciUlpiAccess = (volatile DWORD *)(EhciRegBase + 0xa4);
    DWORD ReadData;
    
    *pEhciUlpiAccess = START_WRITE_ULPI_DATA(Port, Register, Data);

    // wait for register access done
    do 
    {
        ReadData = *pEhciUlpiAccess;
        DEBUGMSG(ZONE_INFO, (TEXT("EHCI ULPI access port %d, register 0x%x = 0x%x\r\n"), Port, Register, ReadData));
    }
    while (ReadData & 0x80000000);
}

void UlpiReadReg(DWORD EhciRegBase, DWORD Port, DWORD Register, BYTE * pData)
{
    volatile DWORD * pEhciUlpiAccess = (volatile DWORD *)(EhciRegBase + 0xa4);
    DWORD ReadData;
    
    *pEhciUlpiAccess = START_READ_ULPI_DATA(Port, Register);

    // wait for register access done
    do 
    {
        ReadData = *pEhciUlpiAccess;
        DEBUGMSG(ZONE_INFO, (TEXT("EHCI ULPI access port %d, register 0x%x = 0x%x\r\n"), Port, Register, ReadData));
    }
    while (ReadData & 0x80000000);
    
    *pData = (BYTE)ReadData;
}
    
BOOL PHY_AssertReset(SEHCDPdd * pPddObject, BOOL bCalledFromPowerUp)
{
    BOOL bRetVal = FALSE;

    // platform specific code (if any) would go here...
        
    DelayMilliSeconds(10, bCalledFromPowerUp);

    bRetVal = TRUE;
    
    return bRetVal;
}

BOOL PHY_DeassertReset(SEHCDPdd * pPddObject)
{
    BOOL bRetVal = FALSE;
    
    // platform specific code (if any) would go here...
        
    bRetVal = TRUE;
    
    return bRetVal;
}


BOOL TLLDevice_AssertReset(SEHCDPdd * pPddObject)
{
    BOOL bRetVal = FALSE;
    PHYSICAL_ADDRESS pa;
    OMAP_PRCM_CORE_PRM_REGS   * pOMAP_PRCM_CORE_PRM_REGS = NULL;
    
    pa.QuadPart         = OMAP_PRCM_CORE_PRM_REGS_PA;
    
    // platform specific code (if any) would go here...

    bRetVal = TRUE;

    return bRetVal;
}


BOOL TLLDevice_DeassertReset(SEHCDPdd * pPddObject)
{
    BOOL bRetVal = FALSE;
    PHYSICAL_ADDRESS pa;
    OMAP_PRCM_CORE_PRM_REGS   * pOMAP_PRCM_CORE_PRM_REGS = NULL;
    
    pa.QuadPart         = OMAP_PRCM_CORE_PRM_REGS_PA;
    
    // platform specific code (if any) would go here...

    bRetVal = TRUE;

    return bRetVal;
}


void ExternalDeviceAssertReset(SEHCDPdd * pPddObject, BOOL bCalledFromPowerUp)
{
    if ((pPddObject->port1Mode == USB_PORT_TLL_MODE) ||
        (pPddObject->port2Mode == USB_PORT_TLL_MODE) ||
        (pPddObject->port3Mode == USB_PORT_TLL_MODE))
    {
       TLLDevice_AssertReset(pPddObject);
    }
    
    if ((pPddObject->port1Mode == USB_PORT_PHY_MODE) ||
        (pPddObject->port2Mode == USB_PORT_PHY_MODE) ||
        (pPddObject->port3Mode == USB_PORT_PHY_MODE))
    {
       PHY_AssertReset(pPddObject, bCalledFromPowerUp);
    }
}

void ExternalDeviceDeassertReset(SEHCDPdd * pPddObject)
{
    if ((pPddObject->port1Mode == USB_PORT_TLL_MODE) ||
        (pPddObject->port2Mode == USB_PORT_TLL_MODE) ||
        (pPddObject->port3Mode == USB_PORT_TLL_MODE))
    {
       TLLDevice_DeassertReset(pPddObject);
    }
    
    if ((pPddObject->port1Mode == USB_PORT_PHY_MODE) ||
        (pPddObject->port2Mode == USB_PORT_PHY_MODE) ||
        (pPddObject->port3Mode == USB_PORT_PHY_MODE))
    {
       PHY_DeassertReset(pPddObject);
    }
}

DWORD DelayExtDevDeassertReset(SEHCDPdd * pPddObject)
{
    Sleep(5000);
    ExternalDeviceDeassertReset(pPddObject);
    Sleep(10);
    DEBUGMSG(ZONE_INFO, (TEXT("EHCI: External device reset deasserted\r\n")));
    return 0;
}

static VOID PortPowerControl(SEHCDPdd * pPddObject, DWORD dwPortNumber, BOOL bPowerOn)
{
    DWORD PortPwrLevel;
    DWORD PortPwrGpio;
    
    switch (dwPortNumber)
    {
        case 1:
            PortPwrLevel = pPddObject->Port1PwrLevel;
            PortPwrGpio = pPddObject->Port1PwrGpio;
            break;

        case 2:
            PortPwrLevel = pPddObject->Port2PwrLevel;
            PortPwrGpio = pPddObject->Port2PwrGpio;
            break;

        case 3:
            PortPwrLevel = pPddObject->Port3PwrLevel;
            PortPwrGpio = pPddObject->Port3PwrGpio;
            break;

        default:
            return;
    }

    if (PortPwrGpio == -1)
        return;

    GPIOSetMode(pPddObject->hGpio, PortPwrGpio, GPIO_DIR_OUTPUT);

    if (bPowerOn)
    {
        if (PortPwrLevel == 0)
            GPIOClrBit(pPddObject->hGpio , PortPwrGpio);
        else
            GPIOSetBit(pPddObject->hGpio , PortPwrGpio);
    }
    else
    {
        if (PortPwrLevel == 0)
            GPIOSetBit(pPddObject->hGpio , PortPwrGpio);
        else
            GPIOClrBit(pPddObject->hGpio , PortPwrGpio);
    }
}

static BOOL Configure_EHCI(SEHCDPdd * pPddObject, BOOL bCalledFromPowerUp)
{
    BOOL rv = TRUE, bPortTLLmode[NUM_PORTS];
    DWORD dwStartTickCount = 0, dwCnt, dwHostConfig = 0;
    PHYSICAL_ADDRESS pa;
    OMAP_PRCM_CORE_CM_REGS      * pOMAP_PRCM_CORE_CM_REGS;
    OMAP_USB_TLL_REGS           * pOMAP_USB_TLL_REGS;
    OMAP_UHH_REGS               * pUHHRegs;
    DWORD StartingTickCount;
    
    DEBUGMSG(ZONE_INFO, (TEXT("EHCI: ConfigureEHCI Called\r\n")));
    DEBUGMSG(ZONE_INFO, (TEXT("*****###  Working TLL code with PHY support.  %s   %s\r\n"), TEXT(__DATE__), TEXT(__TIME__)));
    
    pa.QuadPart = OMAP35XX_UHH_REGS_PA;
    pUHHRegs    =(OMAP_UHH_REGS *)MmMapIoSpace(pa, sizeof(OMAP_UHH_REGS), FALSE);
    
    pa.QuadPart      = OMAP_PRCM_CORE_CM_REGS_PA;
    pOMAP_PRCM_CORE_CM_REGS =(OMAP_PRCM_CORE_CM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_CORE_CM_REGS), FALSE);
    
    pa.QuadPart      = OMAP35XX_USBTLL_REGS_PA;
    pOMAP_USB_TLL_REGS =(OMAP_USB_TLL_REGS*)MmMapIoSpace(pa, sizeof(OMAP_USB_TLL_REGS), FALSE);
    
    // put external USB devices in reset before enabling EHCI controller.  
    // Some USB transceivers require to be held in reset before USB host supplies ULPI clock.
    ExternalDeviceAssertReset(pPddObject, bCalledFromPowerUp);
    
    if (pOMAP_PRCM_CORE_CM_REGS &&  pOMAP_USB_TLL_REGS)
    {
        /* start a soft reset on usb tll*/
        pOMAP_USB_TLL_REGS->USBTLL_SYSCONFIG = USBTLL_SYSCONFIG_SOFTRESET | USBTLL_SYSCONFIG_ENAWAKEUP | USBTLL_SYSCONFIG_SIDLEMODE(SIDLE_IGNORE)| USBTLL_SYSCONFIG_CACTIVITY;
        dwStartTickCount = GetTickCount();
        while (!(pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS & USBTLL_SYSSTATUS_RESETDONE))
        {
            if (GetTickCount() - dwStartTickCount > EHCI_PRCM_TIMEOUT)
            {
                DEBUGMSG(ZONE_INFO, (TEXT("EHCI -  USB_TLL 1 reset Timeout 0x%x\r\n"), pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS));
                break;
            }
            DelayMilliSeconds(1, bCalledFromPowerUp);
        }
        
        pUHHRegs->SYSCONFIG |= UHH_SYSCONFIG_AUTOIDLE | UHH_SYSCONFIG_SOFTRESET;
        while (!(pUHHRegs->SYSSTATUS & UHH_SYSSTATUS_RESETDONE))
        {
            if (GetTickCount() - dwStartTickCount > EHCI_PRCM_TIMEOUT)
            {
                DEBUGMSG(ZONE_INFO, (TEXT("EHCI -  UHH reset Timeout 0x%x\r\n"), pUHHRegs->SYSSTATUS));
                break;
            }
            DelayMilliSeconds(1, bCalledFromPowerUp);
        }
        DEBUGMSG(ZONE_INFO, (TEXT("EHCI -  UHH SYSSTATUS 0x%x\r\n"), pUHHRegs->SYSSTATUS));
        
        /* start a soft reset on usb tll*/
        pOMAP_USB_TLL_REGS->USBTLL_SYSCONFIG = USBTLL_SYSCONFIG_AUTOIDLE | USBTLL_SYSCONFIG_SOFTRESET | 
            USBTLL_SYSCONFIG_ENAWAKEUP | USBTLL_SYSCONFIG_SIDLEMODE(SIDLE_IGNORE) | USBTLL_SYSCONFIG_CACTIVITY;
        
        dwStartTickCount = GetTickCount();
        while (!(pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS & USBTLL_SYSSTATUS_RESETDONE))
        {
            if (GetTickCount() - dwStartTickCount > EHCI_PRCM_TIMEOUT)
            {
                DEBUGMSG(ZONE_INFO, (TEXT("EHCI -  USB_TLL reset Timeout 0x%x\r\n"), pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS));
                break;
            }
            DelayMilliSeconds(1, bCalledFromPowerUp);
        }
        
        pUHHRegs->SYSCONFIG = UHH_SYSCONFIG_ENAWAKEUP | UHH_SYSCONFIG_SIDLEMODE(SIDLE_IGNORE) | 
            UHH_SYSCONFIG_CACTIVITY | UHH_SYSCONFIG_MIDLEMODE(MIDLE_IGNORE);
        
        for (dwCnt = 0; dwCnt < NUM_PORTS; dwCnt++)
            bPortTLLmode[dwCnt] = FALSE;

        if (pPddObject->port1Mode == USB_PORT_TLL_MODE)
        {
            dwHostConfig |= UHH_HOSTCONFIG_P1_ULPI_BYPASS;
            bPortTLLmode[0] = TRUE;
        }
        
        // bypass bits were added for ports 2 and 3
        if (pPddObject->port2Mode == USB_PORT_TLL_MODE)
        {
            dwHostConfig |= UHH_HOSTCONFIG_P2_ULPI_BYPASS;
            bPortTLLmode[1] = TRUE;
        }
        
        if (pPddObject->port3Mode == USB_PORT_TLL_MODE)
        {
            dwHostConfig |= UHH_HOSTCONFIG_P3_ULPI_BYPASS;
            bPortTLLmode[2] = TRUE;
        }

        dwHostConfig |= UHH_HOSTCONFIG_ENA_INCR4 | 
            UHH_HOSTCONFIG_ENA_INCR8 | UHH_HOSTCONFIG_ENA_INCR16;
        
        pUHHRegs->HOSTCONFIG = dwHostConfig;
        
        // Not sure what this polling loop is for...
        if (pPddObject->port1Mode == USB_PORT_PHY_MODE)
        {
            while (pUHHRegs->HOSTCONFIG & UHH_HOSTCONFIG_P1_ULPI_BYPASS);
            
            DEBUGMSG(ZONE_INFO, (TEXT("EHCI: PHY Mode Polling done\r\n")));
        }
        
        if ((pPddObject->port1Mode == USB_PORT_TLL_MODE) ||
            (pPddObject->port2Mode == USB_PORT_TLL_MODE) ||
            (pPddObject->port3Mode == USB_PORT_TLL_MODE) 
            )
        {
            DEBUGMSG(ZONE_INFO, (TEXT("EHCI: TLL Mode Polling start...\r\n")));
            DEBUGMSG(ZONE_INFO, (TEXT("##########USBTLL_SYSSTATUS =0x%x\r\n"), pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS));
            
            for (dwCnt = 0; dwCnt < NUM_PORTS; dwCnt++)
            {
                if (bPortTLLmode[dwCnt] == TRUE)
                    pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[dwCnt] &= 
                             ~(USBTLL_CHANNEL_CONF_i_ULPIDDRMODE | 
                             USBTLL_CHANNEL_CONF_i_UTMIAUTOIDLE | 
                             USBTLL_CHANNEL_CONF_i_ULPIAUTOIDLE);
            }
            
            DEBUGMSG(ZONE_INFO, (TEXT("##########USBTLL_SYSSTATUS =0x%x\r\n"), pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS));
            pOMAP_USB_TLL_REGS->TLL_SHARED_CONF |= USBTLL_SHARED_CONF_FCLK_IS_ON | USBTLL_SHARED_CONF_USB_DIVRATIO(1);
            
            for (dwCnt = 0; dwCnt < NUM_PORTS; dwCnt++)
            {
                if (bPortTLLmode[dwCnt] == TRUE)
                    pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[dwCnt] |= 
                        USBTLL_CHANNEL_CONF_i_CHANEN | USBTLL_CHANNEL_CONF_i_ULPINOBITSTUFF;
            }
        }
        
        if (pPddObject->port1Mode == USB_PORT_PHY_MODE)
        {
            pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[0] = 
                USBTLL_CHANNEL_CONF_i_ULPIOUTCLKMODE | 
                USBTLL_CHANNEL_CONF_i_CHANMODE(2) |
                USBTLL_CHANNEL_CONF_i_CHANEN;
        }

        if (pPddObject->port2Mode == USB_PORT_PHY_MODE)
        {
            pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[1] = 
                USBTLL_CHANNEL_CONF_i_ULPIOUTCLKMODE | 
                USBTLL_CHANNEL_CONF_i_CHANMODE(2) |
                USBTLL_CHANNEL_CONF_i_CHANEN;
        }

        if (pPddObject->port3Mode == USB_PORT_PHY_MODE)
        {
            pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[2] = 
                USBTLL_CHANNEL_CONF_i_ULPIOUTCLKMODE | 
                USBTLL_CHANNEL_CONF_i_CHANMODE(2) |
                USBTLL_CHANNEL_CONF_i_CHANEN;
        }

        MmUnmapIoSpace((PVOID)pOMAP_PRCM_CORE_CM_REGS, sizeof(OMAP_PRCM_CORE_CM_REGS));
        MmUnmapIoSpace((PVOID)pOMAP_USB_TLL_REGS, sizeof(OMAP_USB_TLL_REGS));
        MmUnmapIoSpace((PVOID)pUHHRegs, sizeof(OMAP_UHH_REGS));
    }
    else
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("EHCI - ConfigurePRCMFailed to mmap OMAP_PRCM_USBHOST_CM_REGS_PA")));
        rv = FALSE;
    }
    
    // power up ports
    PortPowerControl(pPddObject, 1, TRUE);
    PortPowerControl(pPddObject, 2, TRUE);
    PortPowerControl(pPddObject, 3, TRUE);

    #if DUMP_REGS
        dumpRegs();
    #endif

    if (!bCalledFromPowerUp)
    {
        CreateThread(NULL, 0, DelayExtDevDeassertReset, pPddObject, 0, 0);
    }
    else
    {
        StartingTickCount = GetTickCount();
        while (GetTickCount() - StartingTickCount < 500)
        {
        }       
        ExternalDeviceDeassertReset(pPddObject);
    }
    
    return rv;
}

/* HcdPdd_DllMain
 *
 *  DLL Entry point.
 *
 * Return Value:
 */
extern BOOL HcdPdd_DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    UnusedParameter(hinstDLL);
    UnusedParameter(dwReason);
    UnusedParameter(lpvReserved);

    return TRUE;
}
/* GetRegistryPhysicalMemSize
 *
 *
 *
 * Return Value:
 */
static BOOL
GetRegistryPhysicalMemSize(
    LPCWSTR RegKeyPath,         // IN - driver registry key path
    DWORD * lpdwPhyscialMemSize)       // OUT - base address
{
    HKEY hKey;
    DWORD dwData;
    DWORD dwSize;
    DWORD dwType;
    BOOL  fRet=FALSE;
    DWORD dwRet;
    
    // Open key
    dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,RegKeyPath,0,0,&hKey);
    if (dwRet != ERROR_SUCCESS) {
        DEBUGMSG(ZONE_ERROR,(TEXT("EHCD:GetRegistryConfig RegOpenKeyEx(%s) failed %d\r\n"),
                             RegKeyPath, dwRet));
        return FALSE;
    }

    // Read base address, range from registry and determine IOSpace
    dwSize = sizeof(dwData);
    dwRet = RegQueryValueEx(hKey, REG_PHYSICAL_PAGE_SIZE, 0, &dwType, (PUCHAR)&dwData, &dwSize);
    if (dwRet == ERROR_SUCCESS) {
        if (lpdwPhyscialMemSize)
            *lpdwPhyscialMemSize = dwData;
        fRet=TRUE;
    }
    RegCloseKey(hKey);
    return fRet;
}



/* InitializeEHCI
 *
 *  Configure and initialize EHCI card
 *
 *  Return Value:
 *  Return TRUE if card could be located and configured, otherwise FALSE
 */
static BOOL
InitializeEHCI(
    SEHCDPdd * pPddObject,    // IN - Pointer to PDD structure
    LPCWSTR szDriverRegKey)   // IN - Pointer to active registry key string
{
    BOOL    fResult = FALSE;
    LPVOID  pobMem = NULL;
    LPVOID  pobEHCD = NULL;
    DWORD   dwHPPhysicalMemSize;
    HKEY    hKey;
    DWORD   dwIrq = IRQ_EHCI;
    PHYSICAL_ADDRESS pa;
    
    DEBUGMSG(ZONE_INFO, (TEXT("InitializeEHCI\r\n")));

    Configure_EHCI(pPddObject, FALSE);
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szDriverRegKey, 0, 0, &hKey)!= ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("InitializeEHCI:GetRegistryConfig RegOpenKeyEx(%s) failed\r\n"), szDriverRegKey));
        return FALSE;
    }
    
    // Map register base to virtual address
    pa.QuadPart = OMAP35XX_EHCI_REGS_PA;
    
    pPddObject->ioPortBase  =(UCHAR *)MmMapIoSpace(pa, OMAP35XX_EHCI_REGS_SIZE, FALSE);
    if (pPddObject->ioPortBase == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("EHCI: Failed to map 0x%x...\r\n"), OMAP35XX_EHCI_REGS_PA));
        return FALSE;
    }
    
    // Request SYSINTR for interrupt
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR,
        &dwIrq,
        sizeof(dwIrq),
        &pPddObject->dwSysIntr,
        sizeof(pPddObject->dwSysIntr),
        NULL))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("EHCI: Failed to aquire IRQ_USB_HGEN")));
        return FALSE;
    }
    
    DEBUGMSG(ZONE_INFO, (L"EHCI: SYSINTR %d\r\n", pPddObject->dwSysIntr));

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // The PDD can supply a buffer of contiguous physical memory here, or can let the
    // MDD try to allocate the memory from system RAM.  We will use the HalAllocateCommonBuffer()
    // API to allocate the memory and bus controller physical addresses and pass this information
    // into the MDD.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    if (GetRegistryPhysicalMemSize(szDriverRegKey, &pPddObject->dwPhysicalMemSize))
    {
        // A quarter for High priority Memory.
        dwHPPhysicalMemSize = pPddObject->dwPhysicalMemSize/4;
        // Align with page size.
        pPddObject->dwPhysicalMemSize =(pPddObject->dwPhysicalMemSize + PAGE_SIZE -1) & ~(PAGE_SIZE -1);
        dwHPPhysicalMemSize =((dwHPPhysicalMemSize +  PAGE_SIZE -1) & ~(PAGE_SIZE -1));
    }
    else
    {
        pPddObject->dwPhysicalMemSize = 0;
    }
    
    if (pPddObject->dwPhysicalMemSize < gcTotalAvailablePhysicalMemory)
    {
        pPddObject->dwPhysicalMemSize = gcTotalAvailablePhysicalMemory;
        dwHPPhysicalMemSize = gcHighPriorityPhysicalMemory;
    }
    
#if (EHCI_DMA_ALLOC)
    // Fill out the DMA adapter descriptor.  The bus address and type is irrelevant
    pPddObject->AdapterObject.ObjectSize    = sizeof(DMA_ADAPTER_OBJECT);
    pPddObject->AdapterObject.InterfaceType = PCIBus;
    pPddObject->AdapterObject.BusNumber     = 0;
    
    
    if ((pPddObject->pvVirtualAddress = HalAllocateCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, &pPddObject->LogicalAddress, FALSE)) == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("EHCI - HalAllocateCommonBuffer Failed 0x%x\r\n"), GetLastError()));
        goto InitializeEHCI_Error;
    }
    
    
    if (!(pobMem = HcdMdd_CreateMemoryObject(pPddObject->dwPhysicalMemSize, dwHPPhysicalMemSize, (PUCHAR) pPddObject->pvVirtualAddress, (PUCHAR) pPddObject->LogicalAddress.LowPart)))
#endif
    if (!(pobMem = HcdMdd_CreateMemoryObject(pPddObject->dwPhysicalMemSize, dwHPPhysicalMemSize, NULL, NULL)))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("EHCI - HcdMdd_CreateMemoryObject Failed 0x%x\r\n"), GetLastError()));
        goto InitializeEHCI_Error;
    }
    
    if (!(pobEHCD = HcdMdd_CreateHcdObject(pPddObject, pobMem, szDriverRegKey, pPddObject->ioPortBase, pPddObject->dwSysIntr)))
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("EHCI - HcdMdd_CreateHcdObject Failed 0x%x\r\n"), GetLastError()));
        goto InitializeEHCI_Error;
    }
    
    pPddObject->lpvMemoryObject  = pobMem;
    pPddObject->lpvEHCDMddObject = pobEHCD;

    _tcsncpy_s(pPddObject->szDriverRegKey,_countof(pPddObject->szDriverRegKey), szDriverRegKey, _tcslen(szDriverRegKey)+1);
    
    if (hKey != NULL)
    {
        DWORD dwCapability;
        DWORD dwType;
        DWORD dwLength = sizeof(DWORD);
        if (RegQueryValueEx(hKey, HCD_CAPABILITY_VALNAME, 0, &dwType, (PUCHAR)&dwCapability, &dwLength) == ERROR_SUCCESS)
        {
            HcdMdd_SetCapability(pobEHCD, dwCapability);
        }
        RegCloseKey(hKey);
    }
    
    ExternalDeviceDeassertReset(pPddObject);
    return TRUE;
    
InitializeEHCI_Error:
    if (pPddObject->IsrHandle)
    {
        FreeIntChainHandler(pPddObject->IsrHandle);
        pPddObject->IsrHandle = NULL;
    }
    
    if (pobEHCD)
        HcdMdd_DestroyHcdObject(pobEHCD);
    if (pobMem)
        HcdMdd_DestroyMemoryObject(pobMem);
    if (pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);
    
    pPddObject->lpvMemoryObject = NULL;
    pPddObject->lpvEHCDMddObject = NULL;
    pPddObject->pvVirtualAddress = NULL;
    if (hKey != NULL)
        RegCloseKey(hKey);
    
    return FALSE;
}

/* HcdPdd_Init
 *
 *   PDD Entry point - called at system init to detect and configure EHCI card.
 *
 * Return Value:
 *   Return pointer to PDD specific data structure, or NULL if error.
 */
extern DWORD
HcdPdd_Init(
    DWORD dwContext)  // IN - Pointer to context value. For device.exe, this is a string
                      //      indicating our active registry key.
{
    SEHCDPdd *  pPddObject = malloc(sizeof(SEHCDPdd));
    BOOL        fRet = FALSE;
    extern DWORD g_dwContext;
    DWORD dwClock = OMAP_DEVICE_USBTLL;

    DEBUGMSG(ZONE_INFO, (TEXT("EHCI USB Host: HcdPdd_Init+\r\n")));

    if (pPddObject)
    {
        pPddObject->pvVirtualAddress = NULL;
        InitializeCriticalSection(&pPddObject->csPdd);
        pPddObject->IsrHandle = NULL;

        // Read device parameters
        if (GetDeviceRegistryParams((LPCWSTR)dwContext, pPddObject, 
            dimof(g_deviceRegParams), g_deviceRegParams) != ERROR_SUCCESS) 
        {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: HcdPdd_Init: "
                L"Failed read registry parameters\r\n"));
            goto cleanUp;
        }

        pPddObject->hParentBusHandle = CreateBusAccessHandle( (LPCTSTR) g_dwContext ) ;
        if (pPddObject->hParentBusHandle == NULL)
            DEBUGMSG(ZONE_ERROR, (TEXT("EHCI: Failure to create bus access handle.  g_dwContext = %s\r\n"), (char *)g_dwContext));

        SetDevicePowerState(pPddObject->hParentBusHandle, D0, NULL);

        // Open GPIO driver
        pPddObject->hGpio = GPIOOpen();
        if (pPddObject->hGpio == INVALID_HANDLE_VALUE) 
            goto cleanUp;

        // open root bus driver
        pPddObject->hRootBus = CreateFile(L"BUS1:", 0, 0, NULL, 0, 0, 0 );
        if (pPddObject->hRootBus == NULL)
        {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: HcdPdd_Init: Failed open root bus driver handle\r\n"));
            goto cleanUp;
        }

        // enable USBTLL clocks
        if (!DeviceIoControl(pPddObject->hRootBus, IOCTL_BUS_REQUEST_CLOCK, &dwClock, sizeof(dwClock), NULL, 0, NULL, NULL))
        {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: HcdPdd_Init: IOCTL_BUS_REQUEST_CLOCK failed\r\n"));
            goto cleanUp;
        }
        Sleep(10);

        fRet = InitializeEHCI(pPddObject, (LPCWSTR)dwContext);

        HcdMdd_SetCapability(pPddObject->lpvEHCDMddObject, 
            /* HCD_SUSPEND_RESUME | */ HCD_ROOT_HUB_INTERRUPT);

        if(!fRet)
        {
            DeleteCriticalSection(&pPddObject->csPdd);
            free(pPddObject);
            pPddObject = NULL;
        }
    }

cleanUp:
    if (!fRet && pPddObject != NULL) 
    {
        if (pPddObject->lpvEHCDMddObject != NULL) 
        HcdMdd_DestroyHcdObject(pPddObject->lpvEHCDMddObject);
        if (pPddObject->lpvMemoryObject != NULL) 
        HcdMdd_DestroyMemoryObject(pPddObject->lpvMemoryObject);
        if (pPddObject->dwSysIntr != 0) 
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, (PVOID)&pPddObject->dwSysIntr, 
            sizeof(&pPddObject->dwSysIntr), NULL, 0, NULL);
        CloseBusAccessHandle(pPddObject->hParentBusHandle);
        free(pPddObject);
        pPddObject = NULL;
    }

    DEBUGMSG(ZONE_INFO, (TEXT("EHCI USB Host: HcdPdd_Init -\r\n")));

    return (DWORD)pPddObject;
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
extern BOOL HcdPdd_CheckConfigPower(
    UCHAR bPort,         // IN - Port number
    DWORD dwCfgPower,    // IN - Power required by configuration
    DWORD dwTotalPower)  // IN - Total power currently in use on port
{
    return ((dwCfgPower + dwTotalPower) > 500) ? FALSE : TRUE;
}

extern void HcdPdd_PowerUp(DWORD hDeviceContext)
{
    SEHCDPdd * pPddObject = (SEHCDPdd *)hDeviceContext;

    #if EHCI_SUSPEND_RESUME_CLOCKS
        DWORD dwClock;

        dwClock = OMAP_DEVICE_USBTLL;
        DeviceIoControl(pPddObject->hRootBus, IOCTL_BUS_REQUEST_CLOCK, &dwClock, sizeof(dwClock), NULL, 0, NULL, NULL);

        SetDevicePowerState(pPddObject->hParentBusHandle, D0, NULL);

        DelayMilliSeconds(10, TRUE);
    #endif

    Configure_EHCI(pPddObject, TRUE);

    HcdMdd_PowerUp(pPddObject->lpvEHCDMddObject);

    // power up ports
    PortPowerControl(pPddObject, 1, TRUE);
    PortPowerControl(pPddObject, 2, TRUE);
    PortPowerControl(pPddObject, 3, TRUE);

    return;
}

extern void HcdPdd_PowerDown(DWORD hDeviceContext)
{
    SEHCDPdd * pPddObject = (SEHCDPdd *)hDeviceContext;
    #if EHCI_SUSPEND_RESUME_CLOCKS
        DWORD dwClock;
    #endif
        
    if (!pPddObject->pwrdownSuspendBus)
    {
        // power down ports
        PortPowerControl(pPddObject, 1, FALSE);
        PortPowerControl(pPddObject, 2, FALSE);
        PortPowerControl(pPddObject, 3, FALSE);
    }
        
    HcdMdd_PowerDown(pPddObject->lpvEHCDMddObject);

    #if EHCI_SUSPEND_RESUME_CLOCKS
        if (pPddObject->pwrdownSuspendBus)
           SetDevicePowerState(pPddObject->hParentBusHandle, D3, NULL);
        else
           SetDevicePowerState(pPddObject->hParentBusHandle, D4, NULL);

        dwClock = OMAP_DEVICE_USBTLL;
        DeviceIoControl(pPddObject->hRootBus, IOCTL_BUS_RELEASE_CLOCK, &dwClock, sizeof(dwClock), NULL, 0, NULL, NULL);
    #endif
        
    return;
}


extern BOOL HcdPdd_Deinit(DWORD hDeviceContext)
{
    SEHCDPdd * pPddObject = (SEHCDPdd *)hDeviceContext;

    if(pPddObject->lpvEHCDMddObject)
        HcdMdd_DestroyHcdObject(pPddObject->lpvEHCDMddObject);
    if(pPddObject->lpvMemoryObject)
        HcdMdd_DestroyMemoryObject(pPddObject->lpvMemoryObject);
    if(pPddObject->pvVirtualAddress)
        HalFreeCommonBuffer(&pPddObject->AdapterObject, pPddObject->dwPhysicalMemSize, pPddObject->LogicalAddress, pPddObject->pvVirtualAddress, FALSE);

    SetDevicePowerState(pPddObject->hParentBusHandle, D4, NULL);
    if (pPddObject->hParentBusHandle)
        CloseBusAccessHandle(pPddObject->hParentBusHandle);

    free(pPddObject);
    return TRUE;
}


extern DWORD HcdPdd_Open(DWORD hDeviceContext, DWORD AccessCode,
        DWORD ShareMode)
{
    UnusedParameter(hDeviceContext);
    UnusedParameter(AccessCode);
    UnusedParameter(ShareMode);

    return (DWORD) (((SEHCDPdd*)hDeviceContext)->lpvEHCDMddObject);
}


extern BOOL HcdPdd_Close(DWORD hOpenContext)
{
    UnusedParameter(hOpenContext);

    return TRUE;
}


extern DWORD HcdPdd_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pBuffer);
    UnusedParameter(Count);

    return (DWORD)-1; // an error occured
}


extern DWORD HcdPdd_Write(DWORD hOpenContext, LPCVOID pSourceBytes,
        DWORD NumberOfBytes)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(pSourceBytes);
    UnusedParameter(NumberOfBytes);

    return (DWORD)-1;
}


extern DWORD HcdPdd_Seek(DWORD hOpenContext, LONG Amount, DWORD Type)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(Amount);
    UnusedParameter(Type);

    return (DWORD)-1;
}


extern BOOL HcdPdd_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn,
        DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
    UnusedParameter(hOpenContext);
    UnusedParameter(dwCode);
    UnusedParameter(pBufIn);
    UnusedParameter(dwLenIn);
    UnusedParameter(pBufOut);
    UnusedParameter(dwLenOut);
    UnusedParameter(pdwActualOut);

    return FALSE;
}


// Manage WinCE suspend/resume events

// This gets called by the MDD's IST when it detects a power resume.
// By default it has nothing to do.
extern void HcdPdd_InitiatePowerUp (DWORD hDeviceContext)
{
    return;
}



#if DUMP_REGS

void dumpRegs(void)
{
    PHYSICAL_ADDRESS pa;
    static OMAP_PRCM_USBHOST_CM_REGS        *pOMAP_USBHOST_CM;
    static OMAP_PRCM_CORE_CM_REGS           *pOMAP_PRCM_CORE_CM_REGS;
    static OMAP_PRCM_USBHOST_PRM_REGS       *pOMAP_PRCM_USBHOST_PRM_REGS;
    static OMAP_PRCM_CLOCK_CONTROL_CM_REGS  *pOMAP_PRCM_CLOCK_CONTROL_CM_REGS;
    static OMAP_USB_TLL_REGS                *pOMAP_USB_TLL_REGS;
    static OMAP_UHH_REGS                    *pUHHRegs;
    static OMAP_SYSC_PADCONFS_REGS          *pConfig;
    static volatile UINT8                   *pbTLL_ULPI, *pReg8;
    UINT32 addr_phy, byte_offset;

/*---------------------------------- CODE ------------------------------------*/
    RETAILMSG(1,(TEXT("EHCI: dumpRegs Called\r\n")));

    RETAILMSG(1,(TEXT("\n\n\n##############################\n")));

    pa.QuadPart      = OMAP35XX_USBTLL_REGS_PA;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0;

    RETAILMSG(1,(TEXT("TLL module:")));

    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x14;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x18;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x1c;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x44;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x48;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);


    pa.QuadPart      = OMAP35XX_UHH_REGS_PA;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0;

    RETAILMSG(1,(TEXT("\nUHH_config module:")));

    byte_offset = 0x0;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x14;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x44;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    MmUnmapIoSpace((PVOID)pReg8, 0x1000);


    pa.QuadPart      = OMAP35XX_EHCI_REGS_PA;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0;
    RETAILMSG(1,(TEXT("\nEHCI module:")));

    for (byte_offset=0; byte_offset <= 0x28; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }


    for (byte_offset=0x50; byte_offset <= 0x5c; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0x90; byte_offset <= 0xa0; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);


    RETAILMSG(1,(TEXT("\nPRCM module:")));

    pa.QuadPart      = 0x48002000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0x274;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x2d8;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    MmUnmapIoSpace((PVOID)pReg8, 0x1000);

    
    pa.QuadPart  = 0x48004000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0x0;

    for (byte_offset=0x0; byte_offset <= 0x4; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0x40; byte_offset <= 0x44; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0x904; byte_offset <= 0x904; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0x940; byte_offset <= 0x944; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }


    for (byte_offset=0xa00; byte_offset <= 0xa08; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0xa10; byte_offset <= 0xa18; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }

    for (byte_offset=0xa30; byte_offset <= 0xa38; byte_offset+=4)
    {
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }


    byte_offset = 0xa40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xb00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xb10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xb40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xc00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xc10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xc30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xc40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xd00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xd04;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    for (byte_offset=0xd40; byte_offset <= 0xd4c; byte_offset+=4)
    {
        RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));
    }


    byte_offset = 0xd50;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xd70;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xe00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xe10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xe30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xe40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xf00;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xf10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xf30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xf40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);




    pa.QuadPart  = 0x48005000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);
    byte_offset = 0x0;

    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x10;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x30;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x40;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x140;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x400;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x410;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x430;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);




    pa.QuadPart  =  0x48306000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);

    byte_offset = 0xae0;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xd70;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);




    pa.QuadPart  =  0x48007000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);

    byte_offset = 0x270;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x4e0;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);



    RETAILMSG(1,(TEXT("\nSDRC module:")));

    pa.QuadPart  = 0x6d000000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);

    byte_offset = 0xa0;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0xa4;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    MmUnmapIoSpace((PVOID)pReg8, 0x1000);



    RETAILMSG(1,(TEXT("\nPAD Conf:")));

    pa.QuadPart  = 0x48002000;
    addr_phy = (UINT32) pa.QuadPart;
    pReg8 = (volatile UINT8*)MmMapIoSpace(pa, 0x1000, FALSE);

    byte_offset = 0x16c;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x170;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x184;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x188;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x164;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x168;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));

    byte_offset = 0x180;
    RETAILMSG(1,(TEXT("[0x%8.8x] (0x%8.8x)"),addr_phy+byte_offset,  *(UINT32*) (pReg8+byte_offset)  ));


    MmUnmapIoSpace((PVOID)pReg8, 0x1000);


    //TLLDevice_AssertReset();
    //  TLLDevice_DeassertReset();

    pa.QuadPart = OMAP35XX_UHH_REGS_PA;
    pUHHRegs    = (OMAP_UHH_REGS *)MmMapIoSpace(pa, OMAP35XX_UHH_REGS_SIZE, FALSE);

    pa.QuadPart      = OMAP_PRCM_USBHOST_CM_REGS_PA;
    pOMAP_USBHOST_CM = (OMAP_PRCM_USBHOST_CM_REGS *)MmMapIoSpace(pa, OMAP_PRCM_USBHOST_CM_REGS_SIZE, FALSE);

    pa.QuadPart      = OMAP_PRCM_CORE_CM_REGS_PA;
    pOMAP_PRCM_CORE_CM_REGS = (OMAP_PRCM_CORE_CM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_CORE_CM_REGS), FALSE);

    pa.QuadPart      = OMAP_PRCM_USBHOST_PRM_REGS_PA;
    pOMAP_PRCM_USBHOST_PRM_REGS = (OMAP_PRCM_USBHOST_PRM_REGS *)MmMapIoSpace(pa, sizeof(OMAP_PRCM_USBHOST_PRM_REGS), FALSE);

    pa.QuadPart      = OMAP_PRCM_CLOCK_CONTROL_CM_REGS_PA;
    pOMAP_PRCM_CLOCK_CONTROL_CM_REGS = (OMAP_PRCM_CLOCK_CONTROL_CM_REGS*)MmMapIoSpace(pa, sizeof(OMAP_PRCM_CLOCK_CONTROL_CM_REGS), FALSE);

    pa.QuadPart      = OMAP35XX_USBTLL_REGS_PA;
    pOMAP_USB_TLL_REGS = (OMAP_USB_TLL_REGS*)MmMapIoSpace(pa, sizeof(OMAP_USB_TLL_REGS), FALSE);

    pa.QuadPart   = OMAP_SYSC_PADCONFS_REGS_PA;
    pConfig       = (OMAP_SYSC_PADCONFS_REGS *)MmMapIoSpace(pa, sizeof(OMAP_SYSC_PADCONFS_REGS), FALSE);

    pa.QuadPart      = OMAP35XX_USBTLL_REGS_PA;
    pbTLL_ULPI = (volatile char*)MmMapIoSpace(pa, 0x1000, FALSE);

    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MMC2_DAT5        (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MMC2_DAT5,  pConfig->CONTROL_PADCONF_MMC2_DAT5  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MMC2_DAT6        (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MMC2_DAT6,  pConfig->CONTROL_PADCONF_MMC2_DAT6  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MMC2_DAT7        (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MMC2_DAT7,  pConfig->CONTROL_PADCONF_MMC2_DAT7  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP3_DX        (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCBSP3_DX,  pConfig->CONTROL_PADCONF_MCBSP3_DX  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP3_DR        (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCBSP3_DR,  pConfig->CONTROL_PADCONF_MCBSP3_DR  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP3_CLKX      (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCBSP3_CLKX,pConfig->CONTROL_PADCONF_MCBSP3_CLKX));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP3_FSX       (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCBSP3_FSX, pConfig->CONTROL_PADCONF_MCBSP3_FSX ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_UART1_CTS        (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_UART1_CTS,  pConfig->CONTROL_PADCONF_UART1_CTS  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP4_CLKX      (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCBSP4_CLKX,pConfig->CONTROL_PADCONF_MCBSP4_CLKX));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP4_DR        (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCBSP4_DR,  pConfig->CONTROL_PADCONF_MCBSP4_DR  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP4_DX        (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCBSP4_DX,  pConfig->CONTROL_PADCONF_MCBSP4_DX  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCBSP4_FSX       (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCBSP4_FSX, pConfig->CONTROL_PADCONF_MCBSP4_FSX ));

    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D10          (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_ETK_D10     ,pConfig->CONTROL_PADCONF_ETK_D10   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D11          (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_ETK_D11     ,pConfig->CONTROL_PADCONF_ETK_D11   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D12          (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_ETK_D12     ,pConfig->CONTROL_PADCONF_ETK_D12   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D13          (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_ETK_D13     ,pConfig->CONTROL_PADCONF_ETK_D13   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D14          (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_ETK_D14     ,pConfig->CONTROL_PADCONF_ETK_D14   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_ETK_D15          (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_ETK_D15     ,pConfig->CONTROL_PADCONF_ETK_D15   ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI1_CS3       (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCSPI1_CS3  ,pConfig->CONTROL_PADCONF_MCSPI1_CS3  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_CS1       (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCSPI2_CS1  ,pConfig->CONTROL_PADCONF_MCSPI2_CS1  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_SIMO      (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCSPI2_SIMO ,pConfig->CONTROL_PADCONF_MCSPI2_SIMO ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_SOMI      (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCSPI2_SOMI ,pConfig->CONTROL_PADCONF_MCSPI2_SOMI ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_CS0       (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCSPI2_CS0  ,pConfig->CONTROL_PADCONF_MCSPI2_CS0  ));
    RETAILMSG(1,(TEXT("CONTROL_PADCONF_MCSPI2_CLK       (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_MCSPI2_CLK  ,pConfig->CONTROL_PADCONF_MCSPI2_CLK  ));

    RETAILMSG(1,(TEXT("CONTROL_PADCONF_UART1_RTS        (0x%x) 0x%x"),&pConfig->CONTROL_PADCONF_UART1_RTS,pConfig->CONTROL_PADCONF_UART1_RTS));


    RETAILMSG(1,(TEXT("CM_FCLKEN3_CORE          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CORE_CM_REGS->CM_FCLKEN3_CORE,   pOMAP_PRCM_CORE_CM_REGS->CM_FCLKEN3_CORE ));
    RETAILMSG(1,(TEXT("CM_ICLKEN3_CORE          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CORE_CM_REGS->CM_ICLKEN3_CORE,   pOMAP_PRCM_CORE_CM_REGS->CM_ICLKEN3_CORE ));
    RETAILMSG(1,(TEXT("CM_AUTOIDLE3_CORE        (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CORE_CM_REGS->CM_AUTOIDLE3_CORE, pOMAP_PRCM_CORE_CM_REGS->CM_AUTOIDLE3_CORE));
    RETAILMSG(1,(TEXT("CM_FCLKEN_USBHOST        (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_FCLKEN_USBHOST, pOMAP_USBHOST_CM->CM_FCLKEN_USBHOST ));
    RETAILMSG(1,(TEXT("CM_ICLKEN_USBHOST        (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_ICLKEN_USBHOST, pOMAP_USBHOST_CM->CM_ICLKEN_USBHOST ));
    RETAILMSG(1,(TEXT("CM_IDLEST_USBHOST        (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_IDLEST_USBHOST, pOMAP_USBHOST_CM->CM_IDLEST_USBHOST ));
    RETAILMSG(1,(TEXT("CM_AUTOIDLE_USBHOST      (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_AUTOIDLE_USBHOST, pOMAP_USBHOST_CM->CM_AUTOIDLE_USBHOST ));
    RETAILMSG(1,(TEXT("CM_SLEEPDEP_USBHOST      (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_SLEEPDEP_USBHOST, pOMAP_USBHOST_CM->CM_SLEEPDEP_USBHOST ));
    RETAILMSG(1,(TEXT("CM_CLKSTCTRL_USBHOST     (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_CLKSTCTRL_USBHOST, pOMAP_USBHOST_CM->CM_CLKSTCTRL_USBHOST ));
    RETAILMSG(1,(TEXT("CM_CLKSTST_USBHOST       (0x%x) 0x%x\r\n"),&pOMAP_USBHOST_CM->CM_CLKSTST_USBHOST, pOMAP_USBHOST_CM->CM_CLKSTST_USBHOST ));


    RETAILMSG(1,(TEXT("RM_RSTST_USBHOST         (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->RM_RSTST_USBHOST     , pOMAP_PRCM_USBHOST_PRM_REGS->RM_RSTST_USBHOST       ));
    RETAILMSG(1,(TEXT("PM_WKEN_USBHOST          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKEN_USBHOST      , pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKEN_USBHOST        ));
    RETAILMSG(1,(TEXT("PM_MPUGRPSEL_USBHOST     (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_MPUGRPSEL_USBHOST , pOMAP_PRCM_USBHOST_PRM_REGS->PM_MPUGRPSEL_USBHOST   ));
    RETAILMSG(1,(TEXT("PM_IVA2GRPSEL_USBHOST    (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_IVA2GRPSEL_USBHOST, pOMAP_PRCM_USBHOST_PRM_REGS->PM_IVA2GRPSEL_USBHOST ));
    RETAILMSG(1,(TEXT("PM_WKST_USBHOST          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKST_USBHOST      , pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKST_USBHOST        ));
    RETAILMSG(1,(TEXT("PM_WKDEP_USBHOST         (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKDEP_USBHOST     , pOMAP_PRCM_USBHOST_PRM_REGS->PM_WKDEP_USBHOST       ));
    RETAILMSG(1,(TEXT("PM_PWSTCTRL_USBHOST      (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTCTRL_USBHOST  , pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTCTRL_USBHOST    ));
    RETAILMSG(1,(TEXT("PM_PWSTST_USBHOST        (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTST_USBHOST    , pOMAP_PRCM_USBHOST_PRM_REGS->PM_PWSTST_USBHOST  ));
    RETAILMSG(1,(TEXT("PM_PREPWSTST_USBHOST     (0x%x) 0x%x\r\n"),&pOMAP_PRCM_USBHOST_PRM_REGS->PM_PREPWSTST_USBHOST , pOMAP_PRCM_USBHOST_PRM_REGS->PM_PREPWSTST_USBHOST   ));

    RETAILMSG(1,(TEXT("CM_CLKEN_PLL             (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKEN_PLL        , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKEN_PLL));
    RETAILMSG(1,(TEXT("CM_CLKEN2_PLL            (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKEN2_PLL       , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKEN2_PLL));
    RETAILMSG(1,(TEXT("CM_IDLEST_CKGEN          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_IDLEST_CKGEN     , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_IDLEST_CKGEN));
    RETAILMSG(1,(TEXT("CM_IDLEST2_CKGEN         (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_IDLEST2_CKGEN    , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_IDLEST2_CKGEN));
    RETAILMSG(1,(TEXT("CM_AUTOIDLE_PLL          (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_AUTOIDLE_PLL     , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_AUTOIDLE_PLL));
    RETAILMSG(1,(TEXT("CM_AUTOIDLE2_PLL         (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_AUTOIDLE2_PLL    , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_AUTOIDLE2_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL1_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL1_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL1_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL2_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL2_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL2_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL3_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL3_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL3_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL4_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL4_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL4_PLL));
    RETAILMSG(1,(TEXT("CM_CLKSEL5_PLL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL5_PLL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKSEL5_PLL));
    RETAILMSG(1,(TEXT("CM_CLKOUT_CTRL           (0x%x) 0x%x\r\n"),&pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKOUT_CTRL      , pOMAP_PRCM_CLOCK_CONTROL_CM_REGS->CM_CLKOUT_CTRL));


    RETAILMSG(1,(TEXT("UHH SYSCONFIG  (0x%x) 0x%x\r\n"),&pUHHRegs->SYSCONFIG, pUHHRegs->SYSCONFIG));
    RETAILMSG(1,(TEXT("UHH SYSSTATUS  (0x%x) 0x%x\r\n"),&pUHHRegs->SYSSTATUS, pUHHRegs->SYSSTATUS));
    RETAILMSG(1,(TEXT("UHH HOSTCONFIG (0x%x) 0x%x\r\n"),&pUHHRegs->HOSTCONFIG,pUHHRegs->HOSTCONFIG));


    RETAILMSG(1,(TEXT("TLL_SHARED_CONF (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->TLL_SHARED_CONF,pOMAP_USB_TLL_REGS->TLL_SHARED_CONF));
    RETAILMSG(1,(TEXT("TLL_CHANNEL_CONF[0] (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[0],pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[0]));
    RETAILMSG(1,(TEXT("TLL_CHANNEL_CONF[1] (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[1],pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[1]));
    RETAILMSG(1,(TEXT("TLL_CHANNEL_CONF[2] (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[2],pOMAP_USB_TLL_REGS->TLL_CHANNEL_CONF[2]));

    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_LO_0 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x800+(0x100*0)),*(pbTLL_ULPI+0x800+(0x100*0))));
    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_LO_1 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x800+(0x100*1)),*(pbTLL_ULPI+0x800+(0x100*1))));
    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_LO_2 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x800+(0x100*2)),*(pbTLL_ULPI+0x800+(0x100*2))));

    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_HI_0 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x801+(0x100*0)),*(pbTLL_ULPI+0x801+(0x100*0))));
    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_HI_1 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x801+(0x100*1)),*(pbTLL_ULPI+0x801+(0x100*1))));
    RETAILMSG(1,(TEXT("TLL ULPI_VENDOR_ID_HI_2 (0x%x) 0x%x\r\n"),(pbTLL_ULPI+0x801+(0x100*2)),*(pbTLL_ULPI+0x801+(0x100*2))));



    RETAILMSG(1,(TEXT("USBTLL_SYSSTATUS (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS,pOMAP_USB_TLL_REGS->USBTLL_SYSSTATUS));
    RETAILMSG(1,(TEXT("USBTLL_SYSCONFIG (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->USBTLL_SYSCONFIG,pOMAP_USB_TLL_REGS->USBTLL_SYSCONFIG));
    RETAILMSG(1,(TEXT("USBTLL_IRQSTATUS (0x%x) 0x%x\r\n"),&pOMAP_USB_TLL_REGS->USBTLL_IRQSTATUS,pOMAP_USB_TLL_REGS->USBTLL_IRQSTATUS));


    MmUnmapIoSpace((PVOID)pUHHRegs, OMAP35XX_UHH_REGS_SIZE);
    MmUnmapIoSpace((PVOID)pOMAP_USBHOST_CM, OMAP_PRCM_USBHOST_CM_REGS_SIZE);
    MmUnmapIoSpace((PVOID)pOMAP_PRCM_CORE_CM_REGS, sizeof(OMAP_PRCM_CORE_CM_REGS));
    MmUnmapIoSpace((PVOID)pOMAP_PRCM_USBHOST_PRM_REGS, sizeof(OMAP_PRCM_USBHOST_PRM_REGS));
    MmUnmapIoSpace((PVOID)pOMAP_PRCM_CLOCK_CONTROL_CM_REGS, sizeof(OMAP_PRCM_CLOCK_CONTROL_CM_REGS));
    MmUnmapIoSpace((PVOID)pOMAP_USB_TLL_REGS, sizeof(OMAP_USB_TLL_REGS));
    MmUnmapIoSpace((PVOID)pConfig, sizeof(OMAP_SYSC_PADCONFS_REGS));
    MmUnmapIoSpace((PVOID)pbTLL_ULPI, 0x1000);
}

#endif // #if DUMP_REGS
