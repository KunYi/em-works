//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_ddk_iomux.c
//
//  This file contains the SoC-specific DDK interface for the IOMUX module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>

#ifdef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
#include <oal.h>
#endif 

#pragma warning(pop)

#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define DDK_GPIO_GET_BANK(x) (x>>5)
#define DDK_GPIO_GET_PIN(x)  (x & 0x1F)
#define DDK_GPIO_GET_BIT(x)  (1 << (x & 0x1F))
#define DDK_GPIO_BANK_DIFF    0x10


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
PVOID pv_HWregPINCTRL = NULL;

//-----------------------------------------------------------------------------
// Local Functions
BOOL IomuxAlloc(void);
BOOL IomuxDealloc(void);

//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetPinMux
//
//  Sets the IOMUX mux for the specified IOMUX pin.
//
//  Parameters:
//      pin
//          [in] Functional pin name used to configure IOMUX HW_PINCTRL_MUXSEL
//
//      muxmode
//          [in] MUX_MODE configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE muxmode)
{
    UINT32 BankNo   = DDK_GPIO_GET_BANK(pin);   // 0..3
    UINT32 BankPin  = DDK_GPIO_GET_PIN(pin);    // 0..31
    UINT32 IoMuxSel;
    UINT32 setMask;
    UINT32 clrMask;

    // Calculate masks
    IoMuxSel = BankNo * 2;              //Convert bank to MUXSEL register

    if( BankPin > 15)
    {
        IoMuxSel++;                    // Goto next register within bank
        BankPin =  BankPin & 0xf;      // Adjust pins down
    }

    setMask = 3 << (BankPin * 2);  // default to mode 3
    clrMask = (~muxmode & 0x3) << (BankPin * 2); // clear proper bits

    // Set both bits so that pin defaults to a hi-impedance input then clear
    // those bits that need to be cleared.

    switch(IoMuxSel)
    {
        case 0:
            HW_PINCTRL_MUXSEL0_SET(setMask);
            HW_PINCTRL_MUXSEL0_CLR(clrMask);
            break;
        case 1:
            HW_PINCTRL_MUXSEL1_SET(setMask);
            HW_PINCTRL_MUXSEL1_CLR(clrMask);
            break;
        case 2:
            HW_PINCTRL_MUXSEL2_SET(setMask);
            HW_PINCTRL_MUXSEL2_CLR(clrMask);
            break;
        case 3:
            HW_PINCTRL_MUXSEL3_SET(setMask);
            HW_PINCTRL_MUXSEL3_CLR(clrMask);
            break;
        case 4:
            HW_PINCTRL_MUXSEL4_SET(setMask);
            HW_PINCTRL_MUXSEL4_CLR(clrMask);
            break;
        case 5:
            HW_PINCTRL_MUXSEL5_SET(setMask);
            HW_PINCTRL_MUXSEL5_CLR(clrMask);
            break;
        case 6:
            HW_PINCTRL_MUXSEL6_SET(setMask);
            HW_PINCTRL_MUXSEL6_CLR(clrMask);
            break;
        case 7:
            HW_PINCTRL_MUXSEL7_SET(setMask);
            HW_PINCTRL_MUXSEL7_CLR(clrMask);
            break;
        case 8:
            HW_PINCTRL_MUXSEL8_SET(setMask);
            HW_PINCTRL_MUXSEL8_CLR(clrMask);
            break;
        case 9:
            HW_PINCTRL_MUXSEL9_SET(setMask);
            HW_PINCTRL_MUXSEL9_CLR(clrMask);
            break;
        case 10:
            HW_PINCTRL_MUXSEL10_SET(setMask);
            HW_PINCTRL_MUXSEL10_CLR(clrMask);
            break;
        case 11:
            HW_PINCTRL_MUXSEL11_SET(setMask);
            HW_PINCTRL_MUXSEL11_CLR(clrMask);
            break;
        case 12:
            HW_PINCTRL_MUXSEL12_SET(setMask);
            HW_PINCTRL_MUXSEL12_CLR(clrMask);
            break;
        case 13:
            HW_PINCTRL_MUXSEL13_SET(setMask);
            HW_PINCTRL_MUXSEL13_CLR(clrMask);
            break;
        default:
            break;
    }

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxGetPinMux
//
//  Gets the IOMUX mux configuration for the specified IOMUX pin.
//
//  Parameters:
//      pin
//          [in] Functional pin name used to select the IOMUX output/input
//               path that will be returned.
//
//      pMuxmode
//          [out] MUX_MODE configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxGetPinMux(DDK_IOMUX_PIN pin, DDK_IOMUX_PIN_MUXMODE *pMuxmode)
{
    UINT32 BankNo   = DDK_GPIO_GET_BANK(pin);   // 0..3
    UINT32 BankPin  = DDK_GPIO_GET_PIN(pin);    // 0..31
    UINT32 IoMuxSel;
    UINT32 Reg = 0;

    // Calculate masks
    IoMuxSel = BankNo * 2;              //Convert bank to MUXSEL register

    if( BankPin > 15)
    {
        IoMuxSel++;                    // Goto next register within bank
        BankPin =  BankPin & 0xf;      // Adjust pins down
    }

    switch( IoMuxSel )
    {
        case 0:
            Reg = HW_PINCTRL_MUXSEL0_RD();
            break;
        case 1:
            Reg = HW_PINCTRL_MUXSEL1_RD();
            break;
        case 2:
            Reg = HW_PINCTRL_MUXSEL2_RD();
            break;
        case 3:
            Reg = HW_PINCTRL_MUXSEL3_RD();
            break;
        case 4:
            Reg = HW_PINCTRL_MUXSEL4_RD();
            break;
        case 5:
            Reg = HW_PINCTRL_MUXSEL5_RD();
            break;
        case 6:
            Reg = HW_PINCTRL_MUXSEL6_RD();
            break;
        case 7:
            Reg = HW_PINCTRL_MUXSEL7_RD();
            break;
        case 8:
            Reg = HW_PINCTRL_MUXSEL8_RD();
            break;
        case 9:
            Reg = HW_PINCTRL_MUXSEL9_RD();
            break;
        case 10:
            Reg = HW_PINCTRL_MUXSEL10_RD();
            break;
        case 11:
            Reg = HW_PINCTRL_MUXSEL11_RD();
            break;
        case 12:
            Reg = HW_PINCTRL_MUXSEL12_RD();
            break;
        case 13:
            Reg = HW_PINCTRL_MUXSEL13_RD();
            break;
        default:
            break;
    }

    *pMuxmode = ((Reg >> (BankPin * 2)) & 0x03);

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxSetPadConfig
//
//  Sets the IOMUX pad configuration for the specified IOMUX pad.
//
//  Parameters:
//      pad
//          [in] Functional pad name used to select the pad that will be
//          configured.
//
//      drive
//          [in] Drive strength configuration.
//
//      pull
//          [in] Pull-up/pull-down/keeper configuration.
//
//      voltage
//          [in] Drive voltage configuration.
//
//      Note that some register does not have support for volatge settings so it
//      is the caller Responsibility to pass DDK_IOMUX_PAD_VOLTAGE_RESERVED when
//    Pin does not have voltage settings.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxSetPadConfig(DDK_IOMUX_PIN pin,
                          DDK_IOMUX_PAD_DRIVE drive,
                          DDK_IOMUX_PAD_PULL pull,
                          DDK_IOMUX_PAD_VOLTAGE voltage)
{
    UINT32 BankNo   = DDK_GPIO_GET_BANK(pin);   // 0..3
    UINT32 BankPin  = DDK_GPIO_GET_PIN(pin);    // 0..31
    UINT32 RegSel,clrMaskDrive,setMaskDrive;
    
    //Pull Up/Keeper  Setting
    if( pull == DDK_IOMUX_PAD_PULL_ENABLE )
    {
        switch( BankNo )
        {
            case 0:
                HW_PINCTRL_PULL0_SET(1 << BankPin);
                break;
            case 1:
                HW_PINCTRL_PULL1_SET(1 << BankPin);
                break;
            case 2:
                HW_PINCTRL_PULL2_SET(1 << BankPin);
                break;
            case 3:
                HW_PINCTRL_PULL3_SET(1 << BankPin);
                break;
            case 4:
                HW_PINCTRL_PULL4_SET(1 << BankPin);
                break;
            case 5:
                HW_PINCTRL_PULL5_SET(1 << BankPin);
                break;
            case 6:
                HW_PINCTRL_PULL6_SET(1 << BankPin);
                break;     
            default:
                break;
        }
    }
    else
    {
        switch( BankNo )
        {
            case 0:
                HW_PINCTRL_PULL0_CLR(1 << BankPin);
                break;
            case 1:
                HW_PINCTRL_PULL1_CLR(1 << BankPin);
                break;
            case 2:
                HW_PINCTRL_PULL2_CLR(1 << BankPin);
                break;
            case 3:
                HW_PINCTRL_PULL3_CLR(1 << BankPin);
                break;
            case 4:
                HW_PINCTRL_PULL4_CLR(1 << BankPin);
                break;
            case 5:
                HW_PINCTRL_PULL5_CLR(1 << BankPin);
                break;
            case 6:
                HW_PINCTRL_PULL6_CLR(1 << BankPin);
                break;
            default:
                break;
        }
    }
    
    // Calculate masks
    // Convert bank to register select
    RegSel  = BankNo * 4;

    // Calcuate the PINCTRL Drive Strength and Voltage Register number
    // based on the pin
    if( BankPin > 23) 
    {
        RegSel += 3;
    }
    else if(BankPin > 15) 
    {
        RegSel += 2;
    }
    else if(BankPin > 7)
    {
        RegSel += 1;
    }
    // adjust pins down
    BankPin = BankPin & 0x7;
    
    // calculate Drive strength and Voltage mask
    clrMaskDrive = 7 << (BankPin * 4);                // default to 4ma/1.8v
    setMaskDrive = (((voltage & 0x1) << 2) | (drive & 0x3)) << (BankPin * 4);    // set proper bits

    // default to  4ma/1.8V
    *(volatile UINT32 *)(HW_PINCTRL_DRIVE0_CLR_ADDR+(DDK_GPIO_BANK_DIFF * RegSel)) = clrMaskDrive;
    
    // set proper bits
    *(volatile UINT32 *)(HW_PINCTRL_DRIVE0_SET_ADDR+(DDK_GPIO_BANK_DIFF * RegSel)) = setMaskDrive;

    return TRUE;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKIomuxGetPadConfig
//
//  Gets the IOMUX pad configration for the specified IOMUX pad.
//
//  Parameters:
//      pad
//          [in] Functional pad name used to select the pad that will be
//          returned.
//
//      pDrive
//          [out] Drive strength configuration.
//
//      pPull
//          [out] Pull-up/pull-down/keeper configuration.
//
//      pVoltage
//          [in] Drive voltage configuration.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKIomuxGetPadConfig(DDK_IOMUX_PIN pin,
                          DDK_IOMUX_PAD_DRIVE   *pDrive,
                          DDK_IOMUX_PAD_PULL    *pPull,
                          DDK_IOMUX_PAD_VOLTAGE *pVoltage)
{
    UINT32 BankNo   = DDK_GPIO_GET_BANK(pin);   // 0..3
    UINT32 BankPin  = DDK_GPIO_GET_PIN(pin);    // 0..31
    UINT32 RegSel;
    UINT32 Reg = 0;

    //Get Pull Up states
    switch( BankNo )
    {
        case 0:
            Reg = HW_PINCTRL_PULL0_RD();
            break;
        case 1:
            Reg = HW_PINCTRL_PULL1_RD();
            break;
        case 2:
            Reg = HW_PINCTRL_PULL2_RD();
            break;
        case 3:
            Reg = HW_PINCTRL_PULL3_RD();
            break;
        case 4:
            Reg = HW_PINCTRL_PULL4_RD();
            break;
        case 5:
            Reg = HW_PINCTRL_PULL5_RD();
            break;
        case 6:
            Reg = HW_PINCTRL_PULL6_RD();
            break;
        default:
            break;
    }

    *pPull = (Reg >> BankPin) & 0x01;
    
    // Calculate masks
    // Convert bank to register select
    RegSel  = BankNo * 4;

    // Calcuate the PINCTRL Drive Strength and Voltage Register number
    // based on the pin
    if( BankPin > 23) 
    {
        RegSel += 3;
    }
    else if(BankPin > 15) 
    {
        RegSel +=2;
    }
    else if(BankPin > 7) 
    {
        RegSel +=1;
    }
    // adjust pins down
    BankPin = BankPin & 0x7;

    // Return the configuration for the pad
    Reg = *(volatile UINT32 *)(HW_PINCTRL_DRIVE0_ADDR + (DDK_GPIO_BANK_DIFF * RegSel));
    *pDrive   = (Reg >> ( BankPin * 4)) & 0x3;
    *pVoltage = ((Reg >> ( BankPin * 4)) & 0x4) >> 2;
    
    return TRUE;
}


//-----------------------------------------------------------------------------
//
//  Function: IomuxAlloc
//
//  This function allocates the data structures required for interaction
//  with the IOMUX hardware.
//
//  Parameters:
//      None
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL IomuxAlloc(void)
{
    BOOL rc = FALSE;

#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
    PHYSICAL_ADDRESS phyAddr;
#endif

    if (pv_HWregPINCTRL == NULL)
    {
#ifdef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
        // Map peripheral physical address to virtual address
        pv_HWregPINCTRL = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_PINCTRL);
#else
        phyAddr.QuadPart = CSP_BASE_REG_PA_PINCTRL;

        // Map peripheral physical address to virtual address
        pv_HWregPINCTRL = (PVOID) MmMapIoSpace(phyAddr, 0x2000, FALSE);
#endif 
        // Check if virtual mapping failed
        if (pv_HWregPINCTRL == NULL)
        {
            ERRORMSG(1, (_T("IomuxAlloc:  Virtual mapping failed!\r\n")));
            goto cleanUp;
        }
        // Turn on Clock for module
        HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_CLKGATE | BM_PINCTRL_CTRL_SFTRST);
    }

    rc = TRUE;

cleanUp:

    if (!rc) IomuxDealloc();

    return rc;
}
//-----------------------------------------------------------------------------
//
//  Function: IomuxDealloc
//
//  This function deallocates the data structures required for interaction
//  with the IOMUX hardware.
//
//  Parameters:
//      None
//
//  Returns:
//      Returns TRUE.
//
//-----------------------------------------------------------------------------
BOOL IomuxDealloc(void)
{
    // Turn off Clock for module
    HW_PINCTRL_CTRL_SET(BM_PINCTRL_CTRL_CLKGATE);

    // Unmap peripheral address space
    if (pv_HWregPINCTRL != NULL)
    {
#ifndef BOOTLOADER_OAL				// CS&ZHL MAR-17-2012: change BOOTLOADER to BOOTLOADER_OAL
        MmUnmapIoSpace(pv_HWregPINCTRL, 0x2000);
#endif
        pv_HWregPINCTRL = NULL;
    }

    return TRUE;
}
