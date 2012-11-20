//-----------------------------------------------------------------------------
//
// Copyright (C) 2009-2010, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
// File:      
//     USBUtils.c
// Purpose:   
//     Maintains misc utils for specific IP
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <Winbase.h>
#include <ceddk.h>
#pragma warning(pop)

#include "bsp.h"
#include "common_usbcommon.h"
//LQK Jul-9-2012
#include "pmu.h"

#ifdef GLOBAL_DEBUG
PUCHAR g_baseMem;
HANDLE g_hUsbInterruptEvent;
DWORD g_dwHostSysIntr;
#endif

#define UTIL_DETAIL 0

extern WORD BSPGetUSBControllerType(void);
extern void DumpUsbClocks(void);
//------------------------------------------------------------------------------
// Function: PowerDownSchemeExist
// 
// Description: This function is called to check if we can stop the system clock
//              and PHY clock, it is determined by IC capability, if VBUS and ID
//              interrupt without clock can wake up the system, we can enter 
//              low power mode, else we can't
//
//------------------------------------------------------------------------------
BOOL PowerDownSchemeExist(void)
{
    return TRUE;
}

//------------------------------------------------------------------------------
// Function: BSPUsbfnDetachNeedRSSwitch
// 
// Description: This function is called to check if we need to close and reopen 
//              USBCMD.RS to put port back to original mode
//              needed in MX28
//
//------------------------------------------------------------------------------
BOOL BSPUsbfnDetachNeedRSSwitch(void)
{
    return TRUE;
}

//------------------------------------------------------------------------------
// Function: BSPUsbXvrEnableVBUSIntr
// 
// Description: This function is called to enable/disable VBUS interrupt
//              We enable USBPHY_ENIRQDEVPLUGIN
//
// Parameters: 
//     regs
//         [IN] Points to USB related registers mapped to virtual address
//     blEnable
//         [IN] TRUE - enable, FALSE - disable
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbXvrEnableVBUSIntr(PUCHAR baseMem, BOOL blEnable)
{
    UNREFERENCED_PARAMETER(baseMem);
    UNREFERENCED_PARAMETER(blEnable);

    // We can't close "HW_DIGCTL_CTRL_USB_CLKGATE" when doing power down
    // so it is not necessary to set any PHY interrupt
}

//------------------------------------------------------------------------------
// Function: BSPUsbXvrEnableIDIntr
// 
// Description: This function is called to enable/disable ID interrupt
//
// Parameters: 
//     regs
//         [IN] Points to USB related registers mapped to virtual address
//     blEnable
//         [IN] TRUE - enable, FALSE - disable
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbXvrEnableIDIntr(PUCHAR baseMem, BOOL blEnable)
{
    UNREFERENCED_PARAMETER(baseMem);
    UNREFERENCED_PARAMETER(blEnable);
}

//------------------------------------------------------------------------------
// Function: BSPUsbConfigurePhyWakeup
// 
// Description: This function is called to enable/disable clock auto generation during
//              during suspend when all clock are gated
//
// Parameters: 
//     idx
//         [IN] specify OTG or HSH1 to operate on
//     blEnable
//         [IN] TRUE - enable, FALSE - disable
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbConfigurePhyWakeup(DWORD idx, BOOL blEnable)
{
    PHYSICAL_ADDRESS phyAddr;
    PVOID pv_HWregUSBPhy0 = NULL, pv_HWregUSBPhy1 = NULL;

    if (pv_HWregUSBPhy0 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY0;
        pv_HWregUSBPhy0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy0 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbPhyStartUp::MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }
    if (pv_HWregUSBPhy1 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY1;
        pv_HWregUSBPhy1 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy1 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbPhyStartUp::MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }

    if (blEnable)
    {
        HW_USBPHY_CTRL_SET(idx, BM_USBPHY_CTRL_ENAUTOSET_USBCLKS);            // bit 26
        HW_USBPHY_CTRL_SET(idx, BM_USBPHY_CTRL_ENAUTOCLR_USBCLKGATE);         // bit 25
        HW_USBPHY_CTRL_SET(idx, BM_USBPHY_CTRL_ENVBUSCHG_WKUP);               // bit 23
        HW_USBPHY_CTRL_SET(idx, BM_USBPHY_CTRL_ENIDCHG_WKUP);                 // bit 22
        HW_USBPHY_CTRL_SET(idx, BM_USBPHY_CTRL_ENDPDMCHG_WKUP);               // bit 21
        HW_USBPHY_CTRL_SET(idx, BP_USBPHY_CTRL_ENAUTOCLR_PHY_PWD);            // bit 20
        HW_USBPHY_CTRL_SET(idx, BM_USBPHY_CTRL_ENAUTOCLR_CLKGATE);            // bit 19
        HW_USBPHY_CTRL_SET(idx, BM_USBPHY_CTRL_ENAUTO_PWRON_PLL);             // bit 18
        HW_USBPHY_CTRL_SET(idx, BM_USBPHY_CTRL_ENIRQWAKEUP);                  // bit 16
    }
    else
    {
        HW_USBPHY_CTRL_CLR(idx, BM_USBPHY_CTRL_ENAUTOSET_USBCLKS);            // bit 26
        HW_USBPHY_CTRL_CLR(idx, BM_USBPHY_CTRL_ENAUTOCLR_USBCLKGATE);         // bit 25
        HW_USBPHY_CTRL_CLR(idx, BM_USBPHY_CTRL_ENVBUSCHG_WKUP);               // bit 23
        HW_USBPHY_CTRL_CLR(idx, BM_USBPHY_CTRL_ENIDCHG_WKUP);                 // bit 22
        HW_USBPHY_CTRL_CLR(idx, BM_USBPHY_CTRL_ENDPDMCHG_WKUP);               // bit 21
        HW_USBPHY_CTRL_CLR(idx, BP_USBPHY_CTRL_ENAUTOCLR_PHY_PWD);            // bit 20
        HW_USBPHY_CTRL_CLR(idx, BM_USBPHY_CTRL_ENAUTOCLR_CLKGATE);            // bit 19
        HW_USBPHY_CTRL_CLR(idx, BM_USBPHY_CTRL_ENAUTO_PWRON_PLL);             // bit 18
        HW_USBPHY_CTRL_CLR(idx, BM_USBPHY_CTRL_ENIRQWAKEUP);                  // bit 16

        HW_USBPHY_CTRL_CLR(idx, BM_USBPHY_CTRL_WAKEUP_IRQ);                   // wakeup IRQ
    }

    if (pv_HWregUSBPhy0) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy0, 0x1000);
    if (pv_HWregUSBPhy1) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy1, 0x1000);
}

//------------------------------------------------------------------------------
// Function: BSPUsbPhyEnterLowPowerMode
// 
// Description: This function is called to enable/disable interrupt available 
//              in low power mode
//
//     blEnable
//         [IN] TRUE - enable, FALSE - disable
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbPhyEnterLowPowerMode(PUCHAR baseMem, BOOL blEnable)
{
    PHYSICAL_ADDRESS phyAddr;
    PVOID pv_HWregUSBPhy0 = NULL, pv_HWregUSBPhy1 = NULL;

    RETAILMSG(UTIL_DETAIL, (L"BSPUsbPhyEnterLowPowerMode blEnable %x\r\n", blEnable));
    if (pv_HWregUSBPhy0 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY0;
        pv_HWregUSBPhy0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy0 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbPhyEnterLowPowerMode MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }
    if (pv_HWregUSBPhy1 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY1;
        pv_HWregUSBPhy1 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy1 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbPhyEnterLowPowerMode MmMapIoSpace failed for pv_HWregUSBPhy1\r\n")));
        }
    }

    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        BSPUsbXvrEnableVBUSIntr(baseMem, blEnable);
        BSPUsbXvrEnableIDIntr(baseMem, blEnable);
        BSPUsbConfigurePhyWakeup(OTGPHY_IDX, blEnable);

        // we also need close PHY clock
        if (blEnable)
        {
            HW_USBPHY_PWD_WR(OTGPHY_IDX, 0xffffffff);
        }
        else
        {
            HW_USBPHY_PWD_WR(OTGPHY_IDX, 0x0);
        }
    }

    if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        BSPUsbXvrEnableVBUSIntr(baseMem, blEnable);
        BSPUsbXvrEnableIDIntr(baseMem, blEnable);
        BSPUsbConfigurePhyWakeup(H1HPHY_IDX, blEnable);

        // we also need close PHY clock
        if (blEnable)
        {
            HW_USBPHY_PWD_WR(H1HPHY_IDX, 0xffffffff);
        }
        else
        {
            HW_USBPHY_PWD_WR(H1HPHY_IDX, 0x0);
        }
    }

    if (pv_HWregUSBPhy0) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy0, 0x1000);
    if (pv_HWregUSBPhy1) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy1, 0x1000);
}

//------------------------------------------------------------------------------
// Function: BSPUsbSetBusConfig
// 
// Description: This function is called to set proper AHB BURST Mode
//
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbSetBusConfig(PUCHAR baseMem)
{
    CSP_USB_REG* pUSBReg = (CSP_USB_REG*)baseMem;
    OUTREG32(&pUSBReg->SBUSCFG, 0x6);
}

//------------------------------------------------------------------------------
// Function: BSPUsbSetCurrentLimitation
// 
// Description: This function is called to set proper current limitaion 
//
// Parameters 
//      bLimitOn : BOOL, true to turn on current limitation, false to turn off limitation
// Return: 
//      NULL
//------------------------------------------------------------------------------
void BSPUsbSetCurrentLimitation(BOOL bLimitOn)
{ 
    PVOID pv_HWregPOWER = NULL;
	UINT32 PowerSource;          //Lqk:Jul-9-2012
    
    PHYSICAL_ADDRESS phyAddr;
    
    PowerSource = 1;
    RETAILMSG(UTIL_DETAIL, (L"BSPUsbSetCurrentLimitation %x\r\n", bLimitOn));
    phyAddr.QuadPart = CSP_BASE_REG_PA_POWER;
    
    // Map peripheral physical address to virtual address
    pv_HWregPOWER = (PVOID) MmMapIoSpace(phyAddr, 0x1000,FALSE);

    if(pv_HWregPOWER == NULL)
    {
        RETAILMSG(TRUE, (L"ERROR:BSPUsbSetCurrentLimitation , pv_HWregPOWER = NULL!\r\n"));
        return;
    }

#ifdef EM9283	//Lqk:Jul-9-2012
		PmuGetPowerSource( &PowerSource );
		if (bLimitOn)
		{
			if( PowerSource )
			{
				// This is in case battery is not present.
				if(HW_POWER_5VCTRL.B.PWDN_5VBRNOUT == 0)
					BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x20); // Set current limit.
			}
		}
		else
		{
			if( PowerSource )
			{   
				// Set current limit to 450mA.    
				BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x24);
			}
			else
			{
				// Set current limit to 780mA.    
				BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x3F);
			}

		}
#else

	if (bLimitOn)
    {
#ifdef BSP_5V_FROM_VBUS
        // This is in case battery is not present.
        if(HW_POWER_5VCTRL.B.PWDN_5VBRNOUT == 0)
            BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x20); // Set current limit.
#endif
    }
    else
    {
#ifdef BSP_5V_FROM_VBUS    
        // Set current limit to 480mA.    
        BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x27);
#else
        // Set current limit to 780mA.    
        BF_WR(POWER_5VCTRL, CHARGE_4P2_ILIMIT, 0x3F);
#endif
    }

#endif    //EM9283

    MmUnmapIoSpace((PVOID)pv_HWregPOWER, 0x1000);
}


//------------------------------------------------------------------------------
// Function: BSPUSBInterruptControl
//
// This function is used to register USB interrupt as system wakeup source or NOT
//
// Parameters:
//          dwIOCTL  -- IOCTL_HAL_ENABLE_WAKE or IOCTL_HAL_DISABLE_WAKE
//          pSysIntr -- pointer to sys interrupt
//          dwLength -- size of pSysIntr
//     
// Returns:
//      N/A
//     
//
//------------------------------------------------------------------------------
void BSPUSBInterruptControl(DWORD dwIOCTL, PVOID pSysIntr, DWORD dwLength)
{
    // only register USB interrupt as system wakeup source when macro
    // USB_WAKEUP_CNANDCN or USB_WAKEUP_CN is defined. 
    RETAILMSG(UTIL_DETAIL, (L"BSPUSBInterruptControl\r\n"));
    if(dwIOCTL == IOCTL_HAL_ENABLE_WAKE)
    {
#if defined USB_WAKEUP_CNANDDN || defined USB_WAKEUP_CN
        KernelIoControl(dwIOCTL, pSysIntr, dwLength, NULL, 0, NULL);
#endif
    }
    else if(dwIOCTL == IOCTL_HAL_DISABLE_WAKE)
    {
        KernelIoControl(dwIOCTL, pSysIntr, dwLength, NULL, 0, NULL);
    }
    return;
}

//-----------------------------------------------------------------------------
//
//  Function: BSPUsbPhyRegDump
//  
//  Description : This function dumps PHY Control Regs for some specific IC
//
//------------------------------------------------------------------------------
void BSPUsbPhyRegDump(void)
{
    PHYSICAL_ADDRESS phyAddr;
    PVOID pv_HWregUSBPhy0 = NULL, pv_HWregUSBPhy1 = NULL;

    RETAILMSG(UTIL_DETAIL, (L"DumpUsbPHY port %s\r\n", BSPGetUSBControllerType() == USB_SEL_OTG ? L"OTG" : L"HSH"));
    if (pv_HWregUSBPhy0 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY0;
        pv_HWregUSBPhy0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy0 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbPhyRegDump::MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }
    if (pv_HWregUSBPhy1 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY1;
        pv_HWregUSBPhy1 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy1 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbPhyRegDump::MmMapIoSpace failed for pv_HWregUSBPhy1\r\n")));
        }
    }

    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        RETAILMSG(1, (L"HW_USBPHY_PWD               %x\r\n", HW_USBPHY_PWD_RD(OTGPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_TX                %x\r\n", HW_USBPHY_TX_RD(OTGPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_RX                %x\r\n", HW_USBPHY_RX_RD(OTGPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_CTRL              %x\r\n", HW_USBPHY_CTRL_RD(OTGPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_STATUS            %x\r\n", HW_USBPHY_STATUS_RD(OTGPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_DEBUG             %x\r\n", HW_USBPHY_DEBUG_RD(OTGPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_VERSION           %x\r\n", HW_USBPHY_VERSION_RD(OTGPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_IP                %x\r\n", HW_USBPHY_IP_RD(OTGPHY_IDX)));
    }
    else if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        RETAILMSG(1, (L"HW_USBPHY_PWD               %x\r\n", HW_USBPHY_PWD_RD(H1HPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_TX                %x\r\n", HW_USBPHY_TX_RD(H1HPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_RX                %x\r\n", HW_USBPHY_RX_RD(H1HPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_CTRL              %x\r\n", HW_USBPHY_CTRL_RD(H1HPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_STATUS            %x\r\n", HW_USBPHY_STATUS_RD(H1HPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_DEBUG             %x\r\n", HW_USBPHY_DEBUG_RD(H1HPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_VERSION           %x\r\n", HW_USBPHY_VERSION_RD(H1HPHY_IDX)));
        RETAILMSG(1, (L"HW_USBPHY_IP                %x\r\n", HW_USBPHY_IP_RD(H1HPHY_IDX)));
    }
    if (pv_HWregUSBPhy0) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy0, 0x1000);
    if (pv_HWregUSBPhy1) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy1, 0x1000);
}

#ifndef DEBUG
//----------------------------------------------------------------
// 
//  Function:  DumpUSBRegs
//
//  Dump all the registers in MX31 usb memory map
//
//  Parameter:
//      pUSBDRegs - pointer to usb memory map
//
//  Example : DumpUSBRegs((PUCHAR)(m_capBase - m_dwOffset))
//
//  Return
//     NULL
//
//----------------------------------------------------------------
void DumpUSBRegs(PUCHAR baseMem)
{
    // offset 0x000 ~ 0x200
    CSP_USB_REG* pUSBReg = (CSP_USB_REG*)baseMem;
    DWORD offset;
    DWORD rangeS = 0x0, rangeE = 0x1f;

    RETAILMSG(1, (L"DumpUSBReg port %s\r\n", BSPGetUSBControllerType() == USB_SEL_OTG ? L"OTG" : L"HSH"));
    RETAILMSG(1, (L"USBCMD  (140h) %x\r\n", INREG32(&pUSBReg->USBCMD)));
    RETAILMSG(1, (L"USBSTS  (144h) %x\r\n", INREG32(&pUSBReg->USBSTS)));
    RETAILMSG(1, (L"USBINTR (148h) %x\r\n", INREG32(&pUSBReg->USBINTR)));
    RETAILMSG(1, (L"PORTSC  (184h) %x\r\n", INREG32(&pUSBReg->PORTSC[0])));
    RETAILMSG(1, (L"OTGSC   (1A4h) %x\r\n", INREG32(&pUSBReg->OTGSC)));
    RETAILMSG(1, (L"SBUSCFG (090h) %x\r\n", INREG32(&pUSBReg->SBUSCFG)));
    RETAILMSG(1, (L"\r\n"));

    RETAILMSG(1, (L"%4x ~ %4x\t", rangeS, rangeE));
    for (offset = 0; offset <= 0x200; offset += 4)
    {
        RETAILMSG(1, (L"%12x", INREG32((PUCHAR)(pUSBReg) + offset)));
        if ((offset + 4) % 32 == 0) 
        {
            RETAILMSG(1, (L"\r\n"));
            rangeS += 32;
            rangeE += 32;
            RETAILMSG(1, (L"%4x ~ %4x\t", rangeS, rangeE));
        }
    }
    RETAILMSG(1, (L"\r\n"));
}
#endif

//----------------------------------------------------------------
// 
//  Function:  DumpUsbExternalRegs
//
//  Dump external registers, such as clock, power, iomux, etc
//
//  Parameter:
//
//  Return
//     NULL
//
//----------------------------------------------------------------
void DumpUsbExternalRegs(void)
{
    // Clocks
    DumpUsbClocks();
}

//-----------------------------------------------------------------------------
//
//  Function: BSPUsbPhyStartUp
//  
//  Description : This function configure to PHY to working mode
//
//  Comment : This function is only called on BSP code so it is not 
//            necessary to implement it on all ICs
//
//------------------------------------------------------------------------------
void BSPUsbPhyStartUp(void)
{
    PHYSICAL_ADDRESS phyAddr;
    PVOID pv_HWregUSBPhy0 = NULL, pv_HWregUSBPhy1 = NULL;

    if (pv_HWregUSBPhy0 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY0;
        pv_HWregUSBPhy0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy0 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbPhyStartUp::MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }
    if (pv_HWregUSBPhy1 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY1;
        pv_HWregUSBPhy1 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy1 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbPhyStartUp::MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }
    /*
     * The reset Algorithm is as 42.4.10 "Correct Way to Soft Reset a Block"
     */

    // Prepare
    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        HW_USBPHY_CTRL_CLR(OTGPHY_IDX, BM_USBPHY_CTRL_SFTRST);
        HW_USBPHY_CTRL_CLR(OTGPHY_IDX, BM_USBPHY_CTRL_CLKGATE);

        // SoftReset
        HW_USBPHY_CTRL_SET(OTGPHY_IDX, BM_USBPHY_CTRL_SFTRST);
        
        // Waiting for confirm
        while (!HW_USBPHY_CTRL(OTGPHY_IDX).B.CLKGATE)
        {
            // busy wait
        }

        // Done
        HW_USBPHY_CTRL_CLR(OTGPHY_IDX, BM_USBPHY_CTRL_SFTRST);
        HW_USBPHY_CTRL_CLR(OTGPHY_IDX, BM_USBPHY_CTRL_CLKGATE);

        /*
         * Other Configurations on PHY
         */
        // not power down all modules
        HW_USBPHY_PWD(OTGPHY_IDX).U = 0;

        HW_USBPHY_CTRL_SET(OTGPHY_IDX, BM_USBPHY_CTRL_ENUTMILEVEL3);
        HW_USBPHY_CTRL_SET(OTGPHY_IDX, BM_USBPHY_CTRL_ENUTMILEVEL2);

        // turn on DEVICE_PLUGIN_DETECT
        HW_USBPHY_CTRL_SET(OTGPHY_IDX, BM_USBPHY_CTRL_ENDEVPLUGINDETECT);
    }
    else if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        HW_USBPHY_CTRL_CLR(H1HPHY_IDX, BM_USBPHY_CTRL_SFTRST);
        HW_USBPHY_CTRL_CLR(H1HPHY_IDX, BM_USBPHY_CTRL_CLKGATE);

        // SoftReset
        HW_USBPHY_CTRL_SET(H1HPHY_IDX, BM_USBPHY_CTRL_SFTRST);
        
        // Waiting for confirm
        while (!HW_USBPHY_CTRL(H1HPHY_IDX).B.CLKGATE)
        {
            // busy wait
        }

        // Done
        HW_USBPHY_CTRL_CLR(H1HPHY_IDX, BM_USBPHY_CTRL_SFTRST);
        HW_USBPHY_CTRL_CLR(H1HPHY_IDX, BM_USBPHY_CTRL_CLKGATE);

        /*
         * Other Configurations on PHY
         */
        // not power down all modules
        HW_USBPHY_PWD(H1HPHY_IDX).U = 0;

        HW_USBPHY_CTRL_SET(H1HPHY_IDX, BM_USBPHY_CTRL_ENUTMILEVEL3);
        HW_USBPHY_CTRL_SET(H1HPHY_IDX, BM_USBPHY_CTRL_ENUTMILEVEL2);

        // turn on DEVICE_PLUGIN_DETECT
        HW_USBPHY_CTRL_SET(H1HPHY_IDX, BM_USBPHY_CTRL_ENDEVPLUGINDETECT);
    }
    if (pv_HWregUSBPhy0) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy0, 0x1000);
    if (pv_HWregUSBPhy1) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy1, 0x1000);
}

//-----------------------------------------------------------------------------
//
//  Function: BSPUsbPhyExit
//  
//  Description : This function release the resource usb phy driver got
//                when host / peripheral driver exit
//
//------------------------------------------------------------------------------
void BSPUsbPhyExit(void)
{
}

//-----------------------------------------------------------------------------
//
//  Function: BSPUSBSwitchModulePower
//  
//  Description : This function switch Power to USB Module 
//
//  Comment : This function is only called on BSP code so it is not 
//            necessary to implement it on all ICs
// 
//------------------------------------------------------------------------------
void BSPUSBSwitchModulePower(BOOL bOn)
{
    UNREFERENCED_PARAMETER(bOn);
}

//-----------------------------------------------------------------------------
//
//  Function: BSPPhyShowDevDiscon
//  
//  Description : This function checks USBPHY if disconnect happens in peripheral mode
//
//                the HW_USBPHY_STATUS_DEVPLUGIN_STATUS gives an 
//                alternative way to check if cable been removed
//  
//  Retrun : TRUE means disconnect
//           FALSE means not disconnect
//
//------------------------------------------------------------------------------
BOOL BSPPhyShowDevDiscon(void)
{
    PHYSICAL_ADDRESS phyAddr;
    PVOID pv_HWregUSBPhy0 = NULL, pv_HWregUSBPhy1 = NULL;
    BOOL ret;

    RETAILMSG(UTIL_DETAIL, (L"BSPPhyShowDevDiscon\r\n"));
    if (pv_HWregUSBPhy0 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY0;
        pv_HWregUSBPhy0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy0 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPPhyShowDevDiscon::MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }
    if (pv_HWregUSBPhy1 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY1;
        pv_HWregUSBPhy1 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy1 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPPhyShowDevDiscon::MmMapIoSpace failed for pv_HWregUSBPhy1\r\n")));
        }
    }

    ret = FALSE;

    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        // For some IC, ENDEVPLUGINDETECT bit may be shutdown by HW, a walkaround is to re-enable 
        // it before check for connect status
        HW_USBPHY_CTRL_SET(OTGPHY_IDX, BM_USBPHY_CTRL_ENDEVPLUGINDETECT);            // bit 4
        while (!(HW_USBPHY_CTRL_RD(OTGPHY_IDX) & BM_USBPHY_CTRL_ENDEVPLUGINDETECT));

        if (HW_USBPHY_STATUS_RD(OTGPHY_IDX) & 0x40)           
        {
            // DEVPLUGIN_STATUS = 1 means connection
            ret = FALSE;
        }
        else                                        
        {
            // DEVPLUGIN_STATUS = 0 means disconnection
            ret = TRUE;
        }
    }
    else if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        HW_USBPHY_CTRL_SET(H1HPHY_IDX, BM_USBPHY_CTRL_ENDEVPLUGINDETECT);            // bit 4
        while (!(HW_USBPHY_CTRL_RD(H1HPHY_IDX) & BM_USBPHY_CTRL_ENDEVPLUGINDETECT));

        if (HW_USBPHY_STATUS_RD(H1HPHY_IDX) & 0x40)           
        {
            // DEVPLUGIN_STATUS = 1 means connection
            ret = FALSE;
        }
        else                                        
        {
            // DEVPLUGIN_STATUS = 0 means disconnection
            ret = TRUE;
        }
    }

    if (pv_HWregUSBPhy0) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy0, 0x1000);
    if (pv_HWregUSBPhy1) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy1, 0x1000);

    return ret;
}

//-----------------------------------------------------------------------------
//  Function: BSPHostDisconDetect
//
//  Description : This function switch USBPHY HS disconnection detect for host
//
//      we should turn on this logic after BUS RESET and turn off
//      this logic after device detach
//
//      turn on this logic before device attach will cause attach failure
//
//  Comment : We can only turn on this logic when the bus works on High Speed Mode
//
//------------------------------------------------------------------------------
void BSPHostDisconDetect(BOOL bOn)
{
    PHYSICAL_ADDRESS phyAddr;
    PVOID pv_HWregUSBPhy0 = NULL, pv_HWregUSBPhy1 = NULL;
    DWORD dwSel = OTGPHY_IDX;

    RETAILMSG(UTIL_DETAIL, (L"BSPHostDisconDetect bOn %d, on port %d\r\n", bOn, BSPGetUSBControllerType()));
    if (pv_HWregUSBPhy0 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY0;
        pv_HWregUSBPhy0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy0 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPHostDisconDetect::MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }
    if (pv_HWregUSBPhy1 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY1;
        pv_HWregUSBPhy1 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy1 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPHostDisconDetect::MmMapIoSpace failed for pv_HWregUSBPhy1\r\n")));
        }
    }

    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        dwSel = OTGPHY_IDX;
    }
    else if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        dwSel = H1HPHY_IDX;
    }

    if (bOn)
    {
        HW_USBPHY_CTRL_SET(dwSel, BM_USBPHY_CTRL_ENHOSTDISCONDETECT);
    }
    else
    {
        HW_USBPHY_CTRL_CLR(dwSel, BM_USBPHY_CTRL_ENHOSTDISCONDETECT);
    }

    if (pv_HWregUSBPhy0) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy0, 0x1000);
    if (pv_HWregUSBPhy1) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy1, 0x1000);
}

//------------------------------------------------------------------------------
// Function: BSPUsbhPutPhySuspend
// 
// Description: we need to do specific work on Phy module 
//              after write PORTSC.SUSP to make usb module real suspend
//
//              Also when resume, we should recover Phy first
//
// Parameters: 
//              baseMem : EHCI register base
//              bOn : TRUE means put phy into suspend
//                    FALSE means recover phy from suspend (resume)
//
// Return: 
//              NA
//------------------------------------------------------------------------------
void BSPUsbhPutPhySuspend(PUCHAR baseMem, BOOL bOn)
{
    CSP_USB_REGS* pUSBDRegs = (CSP_USB_REGS*)baseMem;
    DWORD temp;
    CSP_USB_REG* pReg = (PCSP_USB_REG)(&(pUSBDRegs->OTG));
    WORD sel = OTGPHY_IDX;

    PHYSICAL_ADDRESS phyAddr;
    PVOID pv_HWregUSBPhy0 = NULL, pv_HWregUSBPhy1 = NULL;

    RETAILMSG(UTIL_DETAIL, (L"BSPUsbhPutPhySuspend bOn %d, on port %d\r\n", bOn, BSPGetUSBControllerType()));
    if (pv_HWregUSBPhy0 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY0;
        pv_HWregUSBPhy0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy0 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbhPutPhySuspend::MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }
    if (pv_HWregUSBPhy1 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY1;
        pv_HWregUSBPhy1 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy1 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbhPutPhySuspend::MmMapIoSpace failed for pv_HWregUSBPhy1\r\n")));
        }
    }

    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        pReg = (PCSP_USB_REG)(&(pUSBDRegs->OTG));
        sel = OTGPHY_IDX;
    }
    else if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        pReg = (PCSP_USB_REG)(&(pUSBDRegs->H1));
        sel = H1HPHY_IDX;
    }
    if (bOn)
    {
        // Wait DP/DM goes J
        for (;;)
        {
            temp = INREG32(&pReg->PORTSC);
            if ((temp & (3<<10)) == (2<<10)) 
            {
                break;
            }
        }

        // Enable Resume IRQ detection
        HW_USBPHY_CTRL_SET(sel, BM_USBPHY_CTRL_ENIRQRESUMEDETECT);

        // Power Down Phy Bits
        HW_USBPHY_PWD_WR(sel, 0xffffffff);
    }
    else
    {
        // Disable Resume IRQ
        HW_USBPHY_CTRL_CLR(sel, BM_USBPHY_CTRL_ENIRQRESUMEDETECT);
        HW_USBPHY_CTRL_CLR(sel, BM_USBPHY_CTRL_RESUME_IRQ);

        // Now Power Up the Phy
        HW_USBPHY_PWD_WR(sel, 0x0);
    }
    if (pv_HWregUSBPhy0) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy0, 0x1000);
    if (pv_HWregUSBPhy1) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy1, 0x1000);
}

//------------------------------------------------------------------------------
// Function: BSPUsbhCheckPhyResume
// 
// Description: We use this function to check if PHY module has been
//              put to SUSPEND 
//
//              When HW Interrupt comes, this function return TRUE to mean it 
//              is the first interrupt from suspend, PHY recover should be done 
//              first in this case
//
// Parameters: 
//              NA
//
// Return: 
//              TRUE : means PHY in suspend state
//              FALSE : means PHY not in suspend state
//------------------------------------------------------------------------------
BOOL BSPUsbhCheckPhyResume(void)
{
    PHYSICAL_ADDRESS phyAddr;
    PVOID pv_HWregUSBPhy0 = NULL, pv_HWregUSBPhy1 = NULL;
    WORD sel = OTGPHY_IDX;
    BOOL ret;

    RETAILMSG(UTIL_DETAIL, (L"BSPUsbhCheckPhyResume on port %d\r\n", BSPGetUSBControllerType()));
    if (pv_HWregUSBPhy0 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY0;
        pv_HWregUSBPhy0 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy0 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbhCheckPhyResume::MmMapIoSpace failed for pv_HWregUSBPhy0\r\n")));
        }
    }
    if (pv_HWregUSBPhy1 == NULL)
    {
        phyAddr.QuadPart = CSP_BASE_REG_PA_USBPHY1;
        pv_HWregUSBPhy1 = (PVOID) MmMapIoSpace(phyAddr, 0x1000, FALSE);
        if (pv_HWregUSBPhy1 == NULL)
        {
            RETAILMSG(1, (TEXT("BSPUsbhCheckPhyResume::MmMapIoSpace failed for pv_HWregUSBPhy1\r\n")));
        }
    }
    
    if (BSPGetUSBControllerType() == USB_SEL_OTG)
    {
        sel = OTGPHY_IDX;
    }
    else if (BSPGetUSBControllerType() == USB_SEL_H1)
    {
        sel = H1HPHY_IDX;
    }

    if ((HW_USBPHY_PWD_RD(sel) != 0) && (HW_USBPHY_CTRL(sel).B.ENIRQRESUMEDETECT && HW_USBPHY_CTRL(sel).B.RESUME_IRQ))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }
    if (pv_HWregUSBPhy0) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy0, 0x1000);
    if (pv_HWregUSBPhy1) MmUnmapIoSpace((PVOID)pv_HWregUSBPhy1, 0x1000);

    return ret;
}

//------------------------------------------------------------------------------
// Function: IsResetNeed
// 
// Description: identify if a RESET is needed when Resume
//
// Parameters: 
//
// Return: 
//------------------------------------------------------------------------------
BOOL IsResetNeed(void)
{
    return TRUE;
}

#ifdef GLOBAL_DEBUG
//------------------------------------------------------------------------------
// Function: BSPHostForcePortResume
// 
// Description: Experiment API, currently not used
//
// Parameters: 
//
// Return: 
//------------------------------------------------------------------------------
void BSPHostForcePortResume(void)
{
    CSP_USB_REGS* pUSBDRegs = (CSP_USB_REGS*)g_baseMem;
    SETREG32(&pUSBDRegs->OTG.PORTSC, (1<<6));
}

//------------------------------------------------------------------------------
// Function: BSPUsbTestGPT
// 
// Description: Experiment API, currently not used
//
// Parameters: 
//
// Return: 
//------------------------------------------------------------------------------
void BSPUsbTestGPT(void)
{
    CSP_USB_REGS* pUSBDRegs = (CSP_USB_REGS*)g_baseMem;
    DWORD dw_gpt0_ld, dw_gpt0_ctrl;

    dw_gpt0_ld = INREG32(&pUSBDRegs->OTG.GPTIMER0LD);
    dw_gpt0_ld |= 0xffff;
    OUTREG32(&pUSBDRegs->OTG.GPTIMER0LD, dw_gpt0_ld);
    RETAILMSG(1, (L"GPT0 LD %x\r\n", INREG32(&pUSBDRegs->OTG.GPTIMER0LD)));

    dw_gpt0_ctrl = INREG32(&pUSBDRegs->OTG.GPTIMER0CTRL);
    dw_gpt0_ctrl |= (1<<24); // MODE = 1,  repeat
    dw_gpt0_ctrl |= (1<<31); // RUN = 1
    OUTREG32(&pUSBDRegs->OTG.GPTIMER0CTRL, dw_gpt0_ctrl);
    RETAILMSG(1, (L"GPT0 CTRL %x\r\n", INREG32(&pUSBDRegs->OTG.GPTIMER0CTRL)));
}

//------------------------------------------------------------------------------
// Function: BSPHostInterruptEnable
// 
// Description: Experiment API, currently not used
//
// Parameters: 
//
// Return: 
//------------------------------------------------------------------------------
void BSPHostInterruptEnable(BOOL bOn)
{
    if (bOn)
    {
        InterruptInitialize(g_dwHostSysIntr, g_hUsbInterruptEvent, NULL, 0);
    }
    else
    {
        InterruptDisable(g_dwHostSysIntr);
    }
}
#endif
