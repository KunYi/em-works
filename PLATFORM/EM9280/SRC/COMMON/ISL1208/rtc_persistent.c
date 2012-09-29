//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  Module: rtc_persistent.c
//
//  RTC interface implementations related persistent registers.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#pragma warning(pop)

#include "csp.h"
#include "rtc_persistent.h"

//------------------------------------------------------------------------------
// External Variables
extern PVOID pv_HWregRTC;

//------------------------------------------------------------------------------
// Defines

//------------------------------------------------------------------------------
// Types

//------------------------------------------------------------------------------
// Global Variables

//------------------------------------------------------------------------------
// Local Variables

//bit positions, width and, etc. for persistent bits.
//                      AABBCCDD
//                      AA        Inverted Logic
//                        BB      Bank
//                          CC    Starting Bit
//                            DD  Width
//                                All zeros for not supported
static const UINT32 RtcFieldToMask[][2]=
{
    {RTC_CLOCKSOURCE,            (0x00000001)},   // bank 0, bit 00, width 1
    {RTC_ALARM_WAKE_EN,          (0x00000101)},   // bank 0, bit 01, width 1
    {RTC_ALARM_EN,               (0x00000201)},   // bank 0, bit 02, width 1
    {RTC_LCK_SECS,               (0x00000301)},   // bank 0, bit 03, width 1  
    {RTC_XTAL24MHZ_PDOWN,        (0x80000401)},   // bank 0, bit 04, width 1, inverted logic
    {RTC_XTAL24MHZ_PWRUP,        (0x00000401)},   // bank 0, bit 04, width 1
    {RTC_XTAL32KHZ_PDOWN,        (0x80000501)},   // bank 0, bit 05, width 1, inverted logic
    {RTC_XTAL32KHZ_PWRUP,        (0x00000501)},   // bank 0, bit 05, width 1  
    {RTC_XTAL32_FREQ,            (0x00000601)},   // bank 0, bit 06, width 1
    {RTC_ALARM_WAKE,             (0x00000701)},   // bank 0, bit 07, width 1
    {RTC_MSEC_RES,               (0x00000805)},   // bank 0, bit 08, width 5
    {RTC_DISABLE_XTALSTOP,       (0x80000d01)},   // bank 0, bit 13, width 1, inverted logic
    {RTC_DISABLE_XTALOK,         (0x00000d01)},   // bank 0, bit 13, width 1
    {RTC_LOWER_BIAS,             (0x00000e02)},   // bank 0, bit 14, width 2
    {RTC_DISABLE_PSWITCH,        (0x00001001)},   // bank 0, bit 16, width 1
    {RTC_AUTO_RESTART,           (0x00001101)},   // bank 0, bit 17, width 1
    {RTC_ENABLE_LRADC_PWRUP,     (0x00001201)},   // bank 0, bit 18, width 1
    {RTC_RELEASE_GND,            (0x00001301)},   // bank 0, bit 19, width 1
    {RTC_THERMAL_RESET,          (0x00001401)},   // bank 0, bit 20, width 1
    {RTC_EXTERNAL_RESET,         (0x00001501)},   // bank 0, bit 21, width 1
    {RTC_SPARE_ANALOG,           (0x00001601)},   // bank 0, bit 22, width 6
    {RTC_ADJ_POSLIMITBUCK,       (0x00001C01)},   // bank 0, bit 28, width 4
    {0xffffffff,0xffffffff}
};

//------------------------------------------------------------------------------
// Local Functions
BOOL OALRTC_WritePersistentField(Predefined_PersistentBits field, UINT32 uData);
BOOL OALRTC_ReadPersistentField(Predefined_PersistentBits field, UINT32 *pData);
BOOL OALRTC_ReadPersistent(UINT32 regNum,UINT32 uFieldPos,UINT32 uFieldWidth,BOOL bInvert,UINT32 *pValue);
BOOL OALRTC_WritePersistent(UINT32 regNum,UINT32 uFieldPos,UINT32 uFieldWidth,BOOL bInvert,UINT32 value );

//------------------------------------------------------------------------------
//
//  Function:  OALRTC_ReadRawPersistentField
//
//------------------------------------------------------------------------------
BOOL OALRTC_ReadRawPersistentField(UINT32 uFieldMask, UINT32 *pData)
{
    BOOL rc = FALSE;
    BOOL bInvert;
    UINT32 u32Reg;
    UINT32 u32Bitpos;
    UINT32 u32Width;

    OALMSG(OAL_FUNC, (L"+OALRTC_ReadRawPersistentField\r\n"));

    if(!pData)
    {
        OALMSG(OAL_ERROR, (L"ERROR: OALRTC_ReadRawPersistentField: Invalid parameter\r\n"));
        goto CleanUp;
    }
    
    if(uFieldMask)
    {
        bInvert   =   (uFieldMask & 0x80000000) ? TRUE : FALSE;
        u32Reg    =   (uFieldMask >> 16) & 0xff;
        u32Bitpos =   (uFieldMask >>  8) & 0xff;
        u32Width  =   (uFieldMask      ) & 0xff;

        // validate
        if((!u32Width) || ((u32Width+u32Bitpos) > 32))
        {
            OALMSG(OAL_ERROR, (L"ERROR: OALRTC_ReadRawPersistentField: Invalid parameter\r\n"));
            goto CleanUp;
        }

        rc = OALRTC_ReadPersistent(u32Reg, u32Bitpos, u32Width, bInvert, pData);
    }

CleanUp:
    OALMSG(OAL_FUNC, (L"-OALRTC_ReadRawPersistentField(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALRTC_WriteRawPersistentField
//
//------------------------------------------------------------------------------
BOOL OALRTC_WriteRawPersistentField(UINT32 uFieldMask, UINT32 uData)
{
    BOOL rc = FALSE;
    BOOL bInvert;
    UINT32 u32Reg;
    UINT32 u32Bitpos;
    UINT32 u32Width;

    OALMSG(OAL_FUNC, (L"+OALRTC_WriteRawPersistentField\r\n"));

    if(uFieldMask)
    {
        bInvert   =   (uFieldMask & 0x80000000) ? TRUE : FALSE;
        u32Reg    =   (uFieldMask >> 16) & 0xff;
        u32Bitpos =   (uFieldMask >>  8) & 0xff;
        u32Width  =   (uFieldMask      ) & 0xff;

        // validate
        if((!u32Width) || (u32Width+u32Bitpos > 32))
        {
            OALMSG(OAL_ERROR, (L"ERROR: OALRTC_WriteRawPersistentField: Invalid parameter\r\n"));
            goto CleanUp;
        }
        rc = OALRTC_WritePersistent(u32Reg, u32Bitpos, u32Width, bInvert, uData);
    }

CleanUp:
    OALMSG(OAL_FUNC, (L"-OALRTC_WriteRawPersistentField(rc = %d)\r\n", rc));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  OALRTC_GetMaskFromField
//
//------------------------------------------------------------------------------
static UINT32 OALRTC_GetMaskFromField(UINT32 uField)
{
    UINT32 uMask = 0;
    int i;

    for(i=0;; i++)
    {
        if(RtcFieldToMask[i][0] != 0xffffffff && RtcFieldToMask[i][1] != 0xffffffff) //make sure its not a terminator
        {
            if(RtcFieldToMask[i][0] == uField) //if the field matches, get the mask
            {
                uMask = RtcFieldToMask[i][1];
                break;
            }
        }
        else
        { 
            break;//its a terminator, so break out (and uMask will be 0)
        }
    }
    return uMask;
}

//------------------------------------------------------------------------------
//
//  Function:  OALRTC_ReadPersistentField
//
//------------------------------------------------------------------------------
BOOL OALRTC_ReadPersistentField(Predefined_PersistentBits field, UINT32 *pData)
{
    BOOL rc = FALSE;
    UINT32 mask;

    mask = OALRTC_GetMaskFromField(field);
    if(mask)
    {
        rc = OALRTC_ReadRawPersistentField(mask, pData);
    }
    return rc;
}
//------------------------------------------------------------------------------
//
//  Function:  OALRTC_ReadPersistentField
//
//------------------------------------------------------------------------------
BOOL OALRTC_WritePersistentField(Predefined_PersistentBits field, UINT32 uData)
{
    BOOL rc = FALSE;
    UINT32 mask;

    mask = OALRTC_GetMaskFromField(field);
    if(mask)
    {
        rc = OALRTC_WriteRawPersistentField(mask,uData);
    }
    return rc;
}
//-----------------------------------------------------------------------------
//
// Function: OALRTC_ReadPersistent
//
// Generic Persistent Register read
// Extracts a generic bit or bit field from the specified
// persistent register given a field mask and the position
// of the field within the register
//
// Parameters:
//      regNum
//          [in] The persistent register number 0-5
//      uFieldPos
//          [in] The bit position of the field within the register
//      uFieldWidth
//          [in] Number of bits in the field
//      bInvert
//          [in] Bitwise inverts the input
//      pValue
//          [in][out] Pointer to the u32 to get the value
// Returns:
//      TRUE/FALSE.
//
//-----------------------------------------------------------------------------

BOOL OALRTC_ReadPersistent(UINT32 regNum,
                           UINT32 uFieldPos,
                           UINT32 uFieldWidth,
                           BOOL bInvert,
                           UINT32 *pValue)
{
    BOOL rc = FALSE;
    UINT32 value = 0;

    if(pValue)
    {
        if((uFieldPos + uFieldWidth) <= 32)
        {
            // Begin a shadow register read
            while(HW_RTC_STAT_RD() & BM_RTC_STAT_STALE_REGS);

            switch(regNum)
            {
                case 0:
                    value = HW_RTC_PERSISTENT0_RD();
                    break;
                case 1:
                    value = HW_RTC_PERSISTENT1_RD();
                    break;
                case 2:
                    value = HW_RTC_PERSISTENT2_RD();
                    break;
                case 3:
                    value = HW_RTC_PERSISTENT3_RD();
                    break;
                case 4:
                    value = HW_RTC_PERSISTENT4_RD();
                    break;
                case 5:
                    value = HW_RTC_PERSISTENT5_RD();
                    break;
                default:
                    rc = FALSE;
                    break;
            }

            if(bInvert)
            {
                (value) = ~(value);
            }
            
            *pValue = (value>>uFieldPos) & ((1<<uFieldWidth)-1) ;

            rc = TRUE;
        }
    }
    return rc;
}
//-----------------------------------------------------------------------------
//
// Function: OALRTC_WritePersistent
//
// Generic Persistent Register write
// Writes a generic bit or bit field to the specified
// persistent register given a field mask and the position
// of the field within the register
//
// Parameters:
//      regNum
//          [in] The persistent register number 0-5
//      uFieldPos
//            [in] The bit position of the field within the register
//        uFieldWidth
//            [in] Number of bits in the field
//      Value
//            [in] The value of the bit field to write
// Returns:
//      TRUE/FALSE.
//
//-----------------------------------------------------------------------------
BOOL OALRTC_WritePersistent(UINT32 regNum,
                            UINT32 uFieldPos,
                            UINT32 uFieldWidth,
                            BOOL bInvert,
                            UINT32 value )
{
    BOOL rc = FALSE;
    UINT32 reg;
    UINT32 uFieldMask = ((1 << uFieldWidth) - 1) << uFieldPos;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(bInvert);

    // Read full register
    rc = OALRTC_ReadPersistent(regNum, 0, 32, FALSE, &reg);
    if(rc)
    {
        // Clear the field to be written to
        reg &= ~(uFieldMask);

        // OR in the new value (ensuring it's been masked & shifted properly)
        reg |= ((value << uFieldPos) & uFieldMask);

        // Begin a write to a shadow register
        while(HW_RTC_STAT_RD() & BM_RTC_STAT_NEW_REGS);

        switch(regNum)
        {
            case 0:
                HW_RTC_PERSISTENT0_WR(reg);
                break;
            case 1:
                HW_RTC_PERSISTENT1_WR(reg);
                break;
            case 2:
                HW_RTC_PERSISTENT2_WR(reg);
                break;
            case 3:
                HW_RTC_PERSISTENT3_WR(reg);
                break;
            case 4:
                HW_RTC_PERSISTENT4_WR(reg);
                break;
            case 5:
                HW_RTC_PERSISTENT5_WR(reg);
                break;
            default:
                rc = FALSE;
                break;
        }
        // Finish a write to a shadow register
        while(HW_RTC_STAT_RD() & BM_RTC_STAT_NEW_REGS);
        HW_RTC_CTRL_SET(BM_RTC_CTRL_FORCE_UPDATE);
        HW_RTC_CTRL_CLR(BM_RTC_CTRL_FORCE_UPDATE);
        while(HW_RTC_STAT_RD() & BM_RTC_STAT_STALE_REGS);

    }
    return rc;
}

//------------------------------------------------------------------------------
