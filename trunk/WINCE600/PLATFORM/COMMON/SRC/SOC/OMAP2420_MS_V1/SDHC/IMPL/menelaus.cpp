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
//
// Copyright (c) Intrinsyc Software International.  All rights reserved.
//
//
//------------------------------------------------------------------------------
//
//  File: menelaus.cpp
//
//  Configuration:  
//  Slot 1 : Slot Not powered or enabled until card has been inserted
//  Slot 2: Slot is powered, but not enabled.  When Dat1 Interrupt is triggered, slot is enabled.

//#pragma optimize("", off)         // debug

#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <i2c.h>            // i2c interface
#include <omap2420.h>
#include <bsp_menelaus.h>
#include <oal_intr.h>

#include "menelaus.h"

const GUID DEVICE_IFC_I2C_GUID;

#ifdef DEBUG

#define SDCARD_ZONE_FUNC       DEBUGZONE(11)
#define SDCARD_ZONE_INFO       DEBUGZONE(12)
#define SDCARD_ZONE_INIT       DEBUGZONE(13)
#define SDCARD_ZONE_WARN       DEBUGZONE(14)
#define SDCARD_ZONE_ERROR      DEBUGZONE(15)

#endif

/* ******************************************************* */
/*                                   Object Life Cycle                                  */
/* ******************************************************* */
//------------------------------------------------------------------------------
//  Function:  ctor
//
//  Default Constuctor, create an instance to the i2c bus
//
CMenelaus::CMenelaus(LPCWSTR devicename, DWORD i2cAddress, DWORD i2cAddrSize)
:   m_hI2C(NULL)
,   m_lpsI2CDeviceName(devicename)
,   m_dwI2CMenelausAddress(i2cAddress)
,   m_dwI2CMenelausAddrSize(i2cAddrSize)
,   m_bSlot1(FALSE)
,   m_bSlot2(FALSE)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"ctor CMenelaus(%s, 0x%08x, 0x%08x)\r\n", 
        m_lpsI2CDeviceName, 
        m_dwI2CMenelausAddress, 
        m_dwI2CMenelausAddrSize));

}


//------------------------------------------------------------------------------
//  Function:  dtor
//
//  Default Constuctor
//
CMenelaus::~CMenelaus()
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"dtor CMenelaus\r\n"));
    MenelausDeinit();
}

/* ******************************************************* */
/*                           Public Methods                                              */
/* ******************************************************* */

//------------------------------------------------------------------------------
//  Function:  MenelausInit
//
BOOL 
CMenelaus::MenelausInit()
{
    BOOL rc = FALSE;
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"CMenelaus::MenelausInit: "
        L"Initialize Hardware & Interrupts\r\n"
    ));

    // open I2C bus
    m_hI2C = I2COpen(m_lpsI2CDeviceName);

    if (m_hI2C == NULL) {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CMenelaus::MenelausInit: "
            L"Failed open I2C device %s\r\n", m_lpsI2CDeviceName
        ));
        goto cleanUp;
    }

    if (!I2CSetSlaveAddress(m_hI2C,
                            m_dwI2CMenelausAddrSize,
                            m_dwI2CMenelausAddress)) {
        
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CMenelaus::MenelausInit: "
            L"Failed to set I2C Slave address 0x%08x, size 0x%08x)\r\n", m_dwI2CMenelausAddress, m_dwI2CMenelausAddrSize
        ));

        goto cleanUp;
    }

    // Setup Menelaus Chip
    UCHAR pData = 0x00;
    
    // Clear & Mask MTC Menelaus Interrupts
    if ( !ClearMenelausIrq(IRQ_MENELAUS_CD1) || !MaskMenelausIrq(IRQ_MENELAUS_CD1) ||
         !ClearMenelausIrq(IRQ_MENELAUS_CD2) || !MaskMenelausIrq(IRQ_MENELAUS_CD2) ||
         !ClearMenelausIrq(IRQ_MENELAUS_DL1) || !MaskMenelausIrq(IRQ_MENELAUS_DL1) ||
         !ClearMenelausIrq(IRQ_MENELAUS_DL2) || !MaskMenelausIrq(IRQ_MENELAUS_DL2))
     {
        goto cleanUp;
     }
    
    // confgure VMMC (Power Supply Slot1) for 3.2V
    if (ReadData(MENELAUS_LD0CTRL1_OFFSET, &pData))
    {
        pData &= ~(VMCC_VOLT);  // clear bits
        pData |= VMMC_VOLT32;   // set bits
        
        if (!WriteData(MENELAUS_LD0CTRL1_OFFSET, pData) ||
            !WriteData(MENELAUS_LD0CTRL7_OFFSET, (VMMC_MODE_ON)))
        {
            goto cleanUp;
        }
    }
    else
    {
        goto cleanUp;
    }

    // configure DCDC3 (Power Supply Slot2) for 3.2V
    // turn on DCDC3 suppply to Slot2
    if (ReadData(MENELAUS_DCDCCTRL1_OFFSET, &pData))
    {
        pData &= ~(DCDC3_VOLT);  // clear bits
        pData |= DCDC3_VOLT32;   // set bits
        
        if (!WriteData(MENELAUS_DCDCCTRL1_OFFSET, pData) || 
            !WriteData(MENELAUS_DCDCCTRL3_OFFSET, (DCDC3_MODE_ONPWM)))
        {
            goto cleanUp;
        }
    }
    else
    {
        goto cleanUp;
    }

    // Configure Slots
    // MCT Control Reg 3
    // Slots 1 and slot 2 Auto Power Supply Shutoff Disabled.

    // MCT Control Reg 2
    // DCDC3 Power Supply for Slot2 Selected, buffers and debounce enabled
    // CD1, CD2 buffer enabled
    // S1Dat1, S2Dat1 Buffer enabled (note must be enabled only after power applied to slot)

    // MCT Control Reg 1
    // Push-Pull Buffer
    // Normally Open  Switch
    if (!WriteData(MENELAUS_MCTCTRL3_OFFSET, 0x00) || 
        !WriteData(MENELAUS_MCTCTRL2_OFFSET, (S2D1_BUFEN | S1D1_BUFEN | S2CD_BUFEN | S1CD_BUFEN | VS2_DCDC3)) || 
        !WriteData(MENELAUS_MCTCTRL1_OFFSET, (S2CD_SWNO | S1CD_SWNO)))
    {
        goto cleanUp;
    }
    
    // Unmask Desired Menelaus Interrupts
    UnMaskMenelausIrq(IRQ_MENELAUS_CD1);
    UnMaskMenelausIrq(IRQ_MENELAUS_CD2);
    UnMaskMenelausIrq(IRQ_MENELAUS_DL1);
    UnMaskMenelausIrq(IRQ_MENELAUS_DL2);

#ifdef DEBUG
    DEBUGMSG(SDCARD_ZONE_INFO, (L"Dump Menelaus Configuration! \r\n")); 
    ReadData(MENELAUS_VCORECTRL1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"VCORECTRL1 0x%02X \r\n", pData));
    ReadData(MENELAUS_VCORECTRL2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"VCORECTRL2 0x%02X \r\n", pData));
    ReadData(MENELAUS_VCORECTRL3_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"VCORECTRL3 0x%02X \r\n", pData));
    ReadData(MENELAUS_VCORECTRL4_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"VCORECTRL4 0x%02X \r\n", pData));
    ReadData(MENELAUS_VCORECTRL5_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"VCORECTRL5 0x%02X \r\n", pData));
    ReadData(MENELAUS_DCDCCTRL1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"DCDCCTRL1 0x%02X \r\n", pData));
    ReadData(MENELAUS_DCDCCTRL2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"DCDCCTRL2 0x%02X \r\n", pData));
    ReadData(MENELAUS_DCDCCTRL3_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"DCDCCTRL3 0x%02X \r\n", pData));
    ReadData(MENELAUS_LD0CTRL1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"LD0CTRL1 0x%02X \r\n", pData));
    ReadData(MENELAUS_LD0CTRL2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"LD0CTRL2 0x%02X \r\n", pData));
    ReadData(MENELAUS_LD0CTRL3_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"LD0CTRL3 0x%02X \r\n", pData));
    ReadData(MENELAUS_LD0CTRL4_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"LD0CTRL4 0x%02X \r\n", pData));
    ReadData(MENELAUS_LD0CTRL5_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"LD0CTRL5 0x%02X \r\n", pData));
    ReadData(MENELAUS_LD0CTRL6_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"LD0CTRL6 0x%02X \r\n", pData));
    ReadData(MENELAUS_LD0CTRL7_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"LD0CTRL7 0x%02X \r\n", pData));
    ReadData(MENELAUS_LD0CTRL8_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"LD0CTRL8 0x%02X \r\n", pData));
    ReadData(MENELAUS_SLEEPCTRL1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"SLEEPCTRL1 0x%02X \r\n", pData));
    ReadData(MENELAUS_SLEEPCTRL2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"SLEEPCTRL2 0x%02X \r\n", pData));
    ReadData(MENELAUS_DEVICEOFF_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"DEVICEOFF 0x%02X \r\n", pData));
    ReadData(MENELAUS_OSCCTRL_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"OSCCTRL 0x%02X \r\n", pData));
    ReadData(MENELAUS_DETECTCTRL_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"DETECTCTRL 0x%02X \r\n", pData));
    ReadData(MENELAUS_INTMASK1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"INTMASK1 0x%02X \r\n", pData));
    ReadData(MENELAUS_INTMASK2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"INTMASK2 0x%02X \r\n", pData));
    ReadData(MENELAUS_INTSTATUS1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"INTSTATUS1 0x%02X \r\n", pData));
    ReadData(MENELAUS_INTSTATUS2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"INTSTATUS2 0x%02X \r\n", pData));
    ReadData(MENELAUS_INTACK1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"INTACK1 0x%02X \r\n", pData));
    ReadData(MENELAUS_INTACK2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"INTACK2 0x%02X \r\n", pData));
    ReadData(MENELAUS_GPIOCTRL_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"GPIOCTRL 0x%02X \r\n", pData));
    ReadData(MENELAUS_GPIOIN_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"GPIOIN 0x%02X \r\n", pData));
    ReadData(MENELAUS_GPIOOUT_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"GPIOOUT1 0x%02X \r\n", pData));
    ReadData(MENELAUS_BBSMS_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"BBSMS 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCCTRL_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCCTRL 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCUPDATE_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCUPDATE 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCSEC_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCSEC 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCMIN_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCMIN 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCHR_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCHR 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCDAY_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCDAY 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCMON_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCMON 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCYR_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCYR 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCWKDAY_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCWKDAY 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCALSEC_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCALSEC 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCALMIN_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCALMIN 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCALHR_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCALHR 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCALDAY_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCALDAY 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCALMON_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCALMON 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCALYR_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCALYR 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCCOMPMSB_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCCOMPMSB 0x%02X \r\n", pData));
    ReadData(MENELAUS_RTCCOMPLSB_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"RTCCOMPLSB 0x%02X \r\n", pData));
    ReadData(MENELAUS_S1PULLEN_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"S1PULLEN 0x%02X \r\n", pData));
    ReadData(MENELAUS_S1PULLDIR_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"S1PULLDIR 0x%02X \r\n", pData));
    ReadData(MENELAUS_S2PULLEN_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"S2PULLEN 0x%02X \r\n", pData));
    ReadData(MENELAUS_S2PULLDIR_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"S2PULLDIR 0x%02X \r\n", pData));
    ReadData(MENELAUS_MCTCTRL1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL1 0x%02X \r\n", pData));
    ReadData(MENELAUS_MCTCTRL2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL2 0x%02X \r\n", pData));
    ReadData(MENELAUS_MCTCTRL3_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL3 0x%02X \r\n", pData));
    ReadData(MENELAUS_MCTPINST_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTPINST 0x%02X \r\n", pData));
    ReadData(MENELAUS_DEBOUNCE_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"DEBOUNCE 0x%02X \r\n", pData)); 
    DEBUGMSG(SDCARD_ZONE_INFO, (L"End Dump Menelaus Configuration! \r\n")); 
#endif

    rc = TRUE;

cleanUp:
    if(!rc) {
        // Handle error specific 
        MenelausDeinit();
    }
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  MenelausDeinit
//
BOOL 
CMenelaus::MenelausDeinit()
{
    if (m_hI2C != NULL)
    {
        DEBUGMSG(SDCARD_ZONE_FUNC, (L"CMenelaus::MenelausDeinit: "
            L"De-Initialize Hardware & Interrupts\r\n"
        ));

        // turn off power to slots
        WriteData(MENELAUS_DCDCCTRL3_OFFSET, (DCDC3_MODE_OFF));
        WriteData(MENELAUS_LD0CTRL7_OFFSET, (VMMC_MODE_OFF));
        
        // MCT Control Reg 3
        // disable slots 1 & 2
        WriteData(MENELAUS_MCTCTRL3_OFFSET, 0x00);

        // Clear & Mask MTC Menelaus Interrupts
        ClearMenelausIrq(IRQ_MENELAUS_CD1);
        MaskMenelausIrq(IRQ_MENELAUS_CD1);
        ClearMenelausIrq(IRQ_MENELAUS_CD2);
        MaskMenelausIrq(IRQ_MENELAUS_CD2);
        ClearMenelausIrq(IRQ_MENELAUS_DL1);
        MaskMenelausIrq(IRQ_MENELAUS_DL1);
        ClearMenelausIrq(IRQ_MENELAUS_DL2);
        MaskMenelausIrq(IRQ_MENELAUS_DL2);

        I2CClose(m_hI2C);
        m_hI2C = NULL;

        return TRUE;
    }

    return FALSE;
}

//------------------------------------------------------------------------------
//
//  Function:  SlotsEnabled
//
BOOL 
CMenelaus::SlotsEnabled()
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CMenelaus::SlotsEnabled\r\n"));

    // de-select both slots
    if (!WriteData(MENELAUS_MCTCTRL3_OFFSET, 0x00) ||
        !UnMaskMenelausIrq(IRQ_MENELAUS_CD1) ||
        !UnMaskMenelausIrq(IRQ_MENELAUS_CD2) ||
        !UnMaskMenelausIrq(IRQ_MENELAUS_DL1) ||
        !UnMaskMenelausIrq(IRQ_MENELAUS_DL2))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CMenelaus::SlotsEnabled: "
            L"Failed SlotsEnabled \r\n"
        ));
        return FALSE;
    }

#ifdef DEBUG
    UCHAR pData = 0x00;
    ReadData(MENELAUS_MCTCTRL1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL1 0x%02X \r\n", pData));
    ReadData(MENELAUS_MCTCTRL2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL2 0x%02X \r\n", pData));
    ReadData(MENELAUS_MCTCTRL3_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL3 0x%02X \r\n", pData));
    ReadData(MENELAUS_MCTPINST_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTPINST 0x%02X \r\n", pData));
#endif

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CMenelaus::SlotsEnabled\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  HandleSlot1Insert
//
//  Note: Slot1 will automatically shutdown VMCC when card is removed
//
BOOL 
CMenelaus::HandleSlot1Insert()
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CMenelaus::HandleSlot1Insert\r\n"));

    // set VMCC to ON
    // enable slot 1 and auto shutoff
    if (!WriteData(MENELAUS_MCTCTRL3_OFFSET, (SLOT1_EN)) ||
        !MaskMenelausIrq(IRQ_MENELAUS_DL2) ||
        !MaskMenelausIrq(IRQ_MENELAUS_CD2))
    {
        MenelausDeinit();
        return FALSE;
    }

#if DEBUG_MENELUS == 1
    UCHAR pData = 0x00;
    ReadData(MENELAUS_MCTCTRL1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL1 0x%02X \r\n"), pData));
    ReadData(MENELAUS_MCTCTRL2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL2 0x%02X \r\n"), pData));
    ReadData(MENELAUS_MCTCTRL3_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL3 0x%02X \r\n"), pData));
    ReadData(MENELAUS_MCTPINST_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTPINST 0x%02X \r\n"), pData));
#endif


    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CMenelaus::HandleSlot1Insert\r\n"));
    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:  HandleSlot2Insert
//
//  Note: Slot2 will automatically shutdown VMCC when card is removed
BOOL 
CMenelaus::HandleSlot2Insert()
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CMenelaus::HandleSlot2Insert\r\n"));
    
    // enable slot2
    if (!WriteData(MENELAUS_MCTCTRL3_OFFSET, (SLOT2_EN)) ||
        !MaskMenelausIrq(IRQ_MENELAUS_DL1) ||
        !MaskMenelausIrq(IRQ_MENELAUS_CD1))
    {
        MenelausDeinit();
        return FALSE;
    }

#if DEBUG_MENELUS == 1
    UCHAR pData = 0x00;
    ReadData(MENELAUS_MCTCTRL1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL1 0x%02X \r\n"), pData));
    ReadData(MENELAUS_MCTCTRL2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL2 0x%02X \r\n"), pData));
    ReadData(MENELAUS_MCTCTRL3_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL3 0x%02X \r\n"), pData));
    ReadData(MENELAUS_MCTPINST_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTPINST 0x%02X \r\n"), pData));
#endif 

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CMenelaus::HandleSlot2Insert\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  UpdateSlotsState
//
BOOL
CMenelaus::UpdateSlotsState()
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CMenelaus::UpdateSlotsState\r\n"));

    if (m_hI2C == NULL)
        return FALSE;

    UCHAR pinState = 0x00;
    UCHAR mask = 0x00;
    
    if (!ReadData(MENELAUS_MCTPINST_OFFSET, &pinState) ||
        !ReadData(MENELAUS_INTMASK1_OFFSET, &mask))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CMenelaus::UpdateSlotsState: "
            L"Failed UpdateSlotsState\r\n"
        ));
        return FALSE;
    }
    
    m_bSlot1 = (~pinState & S1_CD_ST) ? TRUE : FALSE;
    
    m_bSlot2 = ((~pinState & S2_DAT1_ST) ||
                (~pinState & S2_CD_ST)) ? TRUE : FALSE;

#if DEBUG_MENELUS == 1
    UCHAR pData = 0x00;
    ReadData(MENELAUS_MCTCTRL1_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL1 0x%02X \r\n"), pData));
    ReadData(MENELAUS_MCTCTRL2_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL2 0x%02X \r\n"), pData));
    ReadData(MENELAUS_MCTCTRL3_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTCTRL3 0x%02X \r\n"), pData));
    ReadData(MENELAUS_MCTPINST_OFFSET, &pData); DEBUGMSG(SDCARD_ZONE_INFO, (L"MCTPINST 0x%02X \r\n"), pData));
#endif

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CMenelaus::UpdateSlotsState\r\n"));
    return TRUE;
}

//------------------------------------------------------------------------------
// Function : AckInterrupts
//
// Acknowledge Menelaus Interrupt
BOOL
CMenelaus::AckInterrupts()
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CMenelaus::AckInterrupts\r\n"));
   
    UCHAR status = 0x00;
    UCHAR mask = 0x00;
    
    if (!ReadData(MENELAUS_INTSTATUS1_OFFSET, &status) ||
        !ReadData(MENELAUS_INTMASK1_OFFSET, &mask))
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CMenelaus::AckInterrupts: "
            L"Failed UpdateSlotsState\r\n"
        ));
        return FALSE;
    }

    DEBUGMSG(SDCARD_ZONE_INFO, (L"CMenelaus::AckInterrupts: "
        L"Menelaus Int Status 0x%02X Int Mask 0x%02X \r\n", status, mask
    ));

    if (status & S1_CD_ST)
        return ClearMenelausIrq(IRQ_MENELAUS_CD1) && UnMaskMenelausIrq(IRQ_MENELAUS_CD1);
    
    if (status & S1_DAT1_ST)
        return ClearMenelausIrq(IRQ_MENELAUS_DL1) && UnMaskMenelausIrq(IRQ_MENELAUS_DL1);

    if (status & S2_CD_ST)
        return ClearMenelausIrq(IRQ_MENELAUS_CD2) && UnMaskMenelausIrq(IRQ_MENELAUS_CD2);

    if (status & S2_DAT1_ST)
        return ClearMenelausIrq(IRQ_MENELAUS_DL2) && UnMaskMenelausIrq(IRQ_MENELAUS_DL2);
        
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CMenelaus::AckInterrupts\r\n"));
    return TRUE;
}

/* ******************************************************* */
/*                           Private Method                                             */
/* ******************************************************* */

//------------------------------------------------------------------------------
//  Function:  ReadData from I2C Bus
//
BOOL 
CMenelaus::ReadData(UCHAR reg, UCHAR *pData)
{
    I2CTRANS trans;
    ZeroMemory(&trans,sizeof(trans));
    trans.mClk_HL_Divisor = I2C_CLOCK_DEFAULT;
    trans.mOpCode[0] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[0] = 0;
    trans.mBuffer[0] = reg;
    trans.mTransLen[0] = 1;
    trans.mOpCode[1] = I2C_OPCODE_READ;
    trans.mBufferOffset[1] = 1;
    trans.mTransLen[1] = 1;
    
    I2CTransact(m_hI2C, &trans);

    if (trans.mErrorCode)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CMenelaus::ReadData: "
            L"Failed with error code 0x%08X\r\n",trans.mErrorCode
        ));
        return FALSE;
    }

    *pData = trans.mBuffer[1];
    return TRUE;
}


//------------------------------------------------------------------------------
//  Function:  WriteData to I2C Bus
//
//
BOOL 
CMenelaus::WriteData(UCHAR reg, UCHAR data)
{
    I2CTRANS trans;
    ZeroMemory(&trans,sizeof(trans));
    trans.mClk_HL_Divisor = I2C_CLOCK_DEFAULT;
    trans.mOpCode[0] = I2C_OPCODE_WRITE;
    trans.mBufferOffset[0] = 0;
    trans.mBuffer[0] = reg;
    trans.mBuffer[1] = data;
    trans.mTransLen[0] = 2;
    
    I2CTransact(m_hI2C, &trans);

    if (trans.mErrorCode)
    {
        DEBUGMSG(SDCARD_ZONE_ERROR, (L"CMenelaus::WriteData: "
            L"Failed with error code 0x%08X\r\n",trans.mErrorCode
        ));
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------------------
// Function : MaskMenelausIrq
//
BOOL 
CMenelaus::MaskMenelausIrq(UINT32 irq)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CMenelaus::MaskMenelausIrq (%d)\r\n", irq));

    BOOL rc = FALSE;
    UCHAR reg;
    UCHAR mask;

    // get the current mask in the Menelaus; first determin which register.
    if ((irq - IRQ_MENELAUS_CD1) < 8)
    {
        reg = MENELAUS_INTMASK1_OFFSET;
                
        rc = ReadData(reg, &mask);

        if (rc)
            mask |= 1 << (irq - IRQ_MENELAUS_CD1); 

        rc =  WriteData(reg, mask);  // Apply Mask
    }
    else if ((irq - IRQ_MENELAUS_RTCTMR) < 5)
    {
        reg = MENELAUS_INTMASK2_OFFSET;

        rc = ReadData(reg, &mask);

        if (rc)
            mask |= 1 << (irq - IRQ_MENELAUS_RTCTMR); 

        rc =  WriteData(reg, mask);  // Apply Mask
    }

    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CMenelaus::MaskMenelausIrq\r\n"));
    return rc;
}

//------------------------------------------------------------------------------
// Function : UnMaskMenelausIrq
//
BOOL 
CMenelaus::UnMaskMenelausIrq(UINT32 irq)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CMenelaus::UnMaskMenelausIrq (%d)\r\n", irq));

    BOOL rc = FALSE;
    UCHAR reg;
    UCHAR mask;
    
    // get the current mask in the Menelaus; first determin which register.
    if ((irq - IRQ_MENELAUS_CD1) < 5)
    {
        reg = MENELAUS_INTMASK1_OFFSET;
                
        rc = ReadData(reg, &mask);

        if (rc)
            mask &= ~(1 << (irq - IRQ_MENELAUS_CD1)); 

        rc =  WriteData(reg, mask);  // Apply Mask
    }
    else if ((irq - IRQ_MENELAUS_RTCTMR) < 5)
    {
        reg = MENELAUS_INTMASK2_OFFSET;

        rc = ReadData(reg, &mask);

        if (rc)
            mask &= ~(1 << (irq - IRQ_MENELAUS_RTCTMR)); 

        rc =  WriteData(reg, mask);  // Apply Mask
    }
    
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CMenelaus::UnMaskMenelausIrq\r\n"));
    return rc;
}

//------------------------------------------------------------------------------
// Function : ClearMenelausIrq
//
BOOL 
CMenelaus::ClearMenelausIrq(UINT32 irq)
{
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"+CMenelaus::ClearMenelausIrq (%d)\r\n", irq));
    BOOL rc = FALSE;

    if ((irq - IRQ_MENELAUS_CD1) < 8)
    {
        rc = WriteData(MENELAUS_INTACK1_OFFSET, (1 << (irq - IRQ_MENELAUS_CD1)));
    }
    else if ((irq - IRQ_MENELAUS_RTCTMR) < 5)
    {
        rc = WriteData(MENELAUS_INTACK2_OFFSET, (1 << (irq - IRQ_MENELAUS_RTCTMR)));
    }
        
    DEBUGMSG(SDCARD_ZONE_FUNC, (L"-CMenelaus::ClearMenelausIrq\r\n"));
    return rc;
}


//#pragma optimize("", on)          // debug

