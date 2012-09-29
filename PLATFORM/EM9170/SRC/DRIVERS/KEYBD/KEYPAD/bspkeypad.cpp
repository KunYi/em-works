//------------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  bspkeypad.c
//
//  Provides BSP-specific configuration routines for the KPP peripheral.
//  Implementation of the keyboard platform dependent scan code to
//  Virtual Key mapping for keyboard driver.
//
//  Exports ScanCodeToVKey for the PDD to use to map scan codes to virtual
//  keys.  A version of this will be needed for every physical/virtual key
//  configuration.
//
//------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <keybddr.h>
#include <laymgr.h>
#include <devicelayout.h>
#pragma warning(pop)

#include "bsp.h"
#include "Keypad.hpp"


//------------------------------------------------------------------------------
// External Functions
extern UINT32 CSPKppGetBaseRegAddr(VOID);


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines
#define ScanCodeTableFirst       0x00
#define ScanCodeTableLast        0x99

#define E0ScanCodeTableFirst    0xE010
#define E0ScanCodeTableLast     0xE07F

#define KEY_DEBOUNCE_PERIOD      40    // msec
#define MAX_KEY_EVENTS           16

#define KPP_COLUMN_INUSE         4 // 8
#define KPP_ROW_INUSE            4 // 8
#define KEY_NUMBER               (KPP_COLUMN_INUSE*KPP_ROW_INUSE)
#define KPP_COLUMN_MASK          ((0x1<<KPP_COLUMN_INUSE) - 1)
#define KPP_ROW_MASK             ((0x1<<KPP_ROW_INUSE) - 1)

//------------------------------------------------------------------------------
// Types
struct VirtualKeyMapping {
    UINT32 uiVk;
    UINT32 uiVkGenerated;
};

//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables
static PCSP_KPP_REGS g_pKPP = NULL;
static volatile BOOL g_SysSuspend = FALSE;
static volatile BOOL g_bRestoreClocks = FALSE;

// This variable is global to this file because it
// will later be used in BSPKppPowerOff.
// Record of which keys were down the last time columns were scanned.
static UINT8 keyDown[KPP_COLUMN_INUSE];
static UINT32 prevDownTime[KEY_NUMBER]; // Time that the key went down.
static BOOL keyDownSent[KEY_NUMBER]; // Tells whether the key down event
                                         // has already been passed up.
                                         // Reset to false after key goes up.
static UINT8 kppStatus[KPP_COLUMN_INUSE]; // Row data from the last time
                                              // columns were scanned.

// Copies of scan sequence variables used to store and restore
// key data in case system enters suspend state

//a KPP_COLUMN_INUSE x KPP_ROW_INUSE maxtrix
static UINT32 IntermediateScanCode[]=
{
//      0x00
    0x95,   //  VK_UP
    0x97,   //  VK_RIGHT
    0x26,   //  '3'
    0x3D,   //  '7'

//      0x04
    0x98,   //  VK_DOWN
    0x96,   //  VK_LEFT
    0x1E,   //  '2'
    0x36,   //  '6'

//      0x08
    0x66,   //  VK_BACK
    0x5A,   //  VK_RETURN
    0x16,   //  '1'
    0x2E,   //  '5'

//      0x0C
    0x29,   //  VK_SPACE
    0x0D,   //  VK_TAB
    0x45,   //  '0'
    0x25,   //  '4'
};

static WORD g_wKPSRStored;
static UINT8 keyDownStored[KPP_COLUMN_INUSE];
static UINT32 prevDownTimeStored[KEY_NUMBER];
static BOOL keyDownSentStored[KEY_NUMBER];
static UINT8 kppStatusStored[KPP_COLUMN_INUSE];


static UINT8 ScanCodeToVKeyTable[] =
{
#ifdef PLAT_PMC
    VK_RETURN,          //SW60
#else
    VK_TACTION,         // Scan Code 0x0
#endif
    VK_F9,              // Scan Code 0x1
    VK_T3,              // Scan Code 0x2
    VK_F5,              // Scan Code 0x3
    VK_F3,              // Scan Code 0x4
    VK_F1,              // Scan Code 0x5
    VK_F2,              // Scan Code 0x6
    VK_F12,             // Scan Code 0x7
    VK_F13,             // Scan Code 0x8
    VK_F10,             // Scan Code 0x9
    VK_F8,              // Scan Code 0xA
    VK_F6,              // Scan Code 0xB
    VK_F4,              // Scan Code 0xC
    VK_TAB,             // Scan Code 0xD
    VK_BACKQUOTE,       // Scan Code 0xE
    VK_CLEAR,           // Scan Code 0xF
    VK_F14,             // Scan Code 0x10
    VK_LMENU,           // Scan Code 0x11
    VK_LSHIFT,          // Scan Code 0x12
    VK_DBE_HIRAGANA,    // Scan Code 0x13
    VK_LCONTROL,        // Scan Code 0x14
    'Q',                // Scan Code 0x15
    '1',                // Scan Code 0x16
    0,              // Scan Code 0x17
    VK_F15,             // Scan Code 0x18
    0,              // Scan Code 0x19
    'Z',                // Scan Code 0x1A
    'S',                // Scan Code 0x1B
    'A',                // Scan Code 0x1C
    'W',                // Scan Code 0x1D
    '2',                // Scan Code 0x1E
    0xf1,               // Scan Code 0x1F
    VK_F16,             // Scan Code 0x20
    'C',                // Scan Code 0x21
    'X',                // Scan Code 0x22
    'D',                // Scan Code 0x23
    'E',                // Scan Code 0x24
    '4',                // Scan Code 0x25
    '3',                // Scan Code 0x26
    0,              // Scan Code 0x27
    VK_F17,             // Scan Code 0x28
    VK_SPACE,           // Scan Code 0x29
    'V',                // Scan Code 0x2A
    'F',                // Scan Code 0x2B
    'T',                // Scan Code 0x2C
    'R',                // Scan Code 0x2D
    '5',                // Scan Code 0x2E
    VK_DBE_FLUSHSTRING, // Scan Code 0x2F
    VK_F18,             // Scan Code 0x30
    'N',                // Scan Code 0x31
    'B',                // Scan Code 0x32
    'H',                // Scan Code 0x33
    'G',                // Scan Code 0x34
    'Y',                // Scan Code 0x35
    '6',                // Scan Code 0x36
    VK_DBE_ROMAN,       // Scan Code 0x37
    VK_F19,             // Scan Code 0x38
    0,              // Scan Code 0x39
    'M',                // Scan Code 0x3A
    'J',                // Scan Code 0x3B
    'U',                // Scan Code 0x3C
    '7',                // Scan Code 0x3D
    '8',                // Scan Code 0x3E
    VK_DBE_SBCSCHAR,    // Scan Code 0x3F
    VK_F20,             // Scan Code 0x40
    VK_COMMA,           // Scan Code 0x41
    'K',                // Scan Code 0x42
    'I',                // Scan Code 0x43
    'O',                // Scan Code 0x44
    '0',                // Scan Code 0x45
    '9',                // Scan Code 0x46
    0,              // Scan Code 0x47
    VK_F21,             // Scan Code 0x48
    VK_PERIOD,          // Scan Code 0x49
    VK_SLASH,           // Scan Code 0x4A
    'L',                // Scan Code 0x4B
    VK_SEMICOLON,       // Scan Code 0x4C
    'P',                // Scan Code 0x4D
    VK_HYPHEN,          // Scan Code 0x4E
    0,              // Scan Code 0x4F
    VK_F22,             // Scan Code 0x50
    VK_EXTEND_BSLASH,   // Scan Code 0x51
    VK_APOSTROPHE,      // Scan Code 0x52
    0,              // Scan Code 0x53
    VK_LBRACKET,        // Scan Code 0x54
    VK_EQUAL,           // Scan Code 0x55
    VK_DBE_NOCODEINPUT, // Scan Code 0x56
    VK_F23,             // Scan Code 0x57
    VK_CAPITAL,         // Scan Code 0x58
    VK_RSHIFT,          // Scan Code 0x59
    VK_RETURN,          // Scan Code 0x5A
    VK_RBRACKET,        // Scan Code 0x5B
    0,              // Scan Code 0x5C
    VK_BACKSLASH,       // Scan Code 0x5D
    VK_HELP,            // Scan Code 0x5E
    VK_F24,             // Scan Code 0x5F
    0,              // Scan Code 0x60
    VK_EXTEND_BSLASH,   // Scan Code 0x61
    0,              // Scan Code 0x62
    0,              // Scan Code 0x63
    VK_CONVERT,         // Scan Code 0x64
    0,              // Scan Code 0x65
    VK_BACK,            // Scan Code 0x66
    VK_NOCONVERT,       // Scan Code 0x67
    VK_TAB,             // Scan Code 0x68
    VK_NUMPAD1,         // Scan Code 0x69
    VK_DBE_ALPHANUMERIC,// Scan Code 0x6A
    VK_NUMPAD4,         // Scan Code 0x6B
    VK_NUMPAD7,         // Scan Code 0x6C
    0,              // Scan Code 0x6D
    0,              // Scan Code 0x6E
    0,              // Scan Code 0x6F
    VK_NUMPAD0,         // Scan Code 0x70
    VK_DECIMAL,         // Scan Code 0x71
    VK_NUMPAD2,         // Scan Code 0x72
    VK_NUMPAD5,         // Scan Code 0x73
    VK_NUMPAD6,         // Scan Code 0x74
    VK_NUMPAD8,         // Scan Code 0x75
    VK_ESCAPE,          // Scan Code 0x76
    VK_NUMLOCK,         // Scan Code 0x77
    VK_F11,             // Scan Code 0x78
    VK_ADD,             // Scan Code 0x79
    VK_NUMPAD3,         // Scan Code 0x7A
    VK_SUBTRACT,        // Scan Code 0x7B
    VK_MULTIPLY,        // Scan Code 0x7C
    VK_NUMPAD9,         // Scan Code 0x7D
    VK_SCROLL,          // Scan Code 0x7E
    0,              // Scan Code 0x7F
    0,              // Scan Code 0x80
    0,              // Scan Code 0x81
    0,              // Scan Code 0x82
    VK_F7,              // Scan Code 0x83
    VK_SNAPSHOT,        // Scan Code 0x84
    0,              // Scan Code 0x85
    0,              // Scan Code 0x86
    0,              // Scan Code 0x87
    0,              // Scan Code 0x88
    0,              // Scan Code 0x89
    0,              // Scan Code 0x8A
    0,              // Scan Code 0x8B
    0,              // Scan Code 0x8C
    0,              // Scan Code 0x8D
    0,              // Scan Code 0x8E
    0,              // Scan Code 0x8F
#ifdef PLAT_PMC
    VK_F1,              //SW8
#else
    VK_APP1,            // Scan Code 0x90
#endif
#ifdef PLAT_PMC
    VK_F2,              //SW6
#else
    VK_APP2,            // Scan Code 0x91
    #endif
#ifdef PLAT_PMC
    VK_F3,              //SW5
#else
    VK_APP3,            //SW5 // Scan Code 0x92
#endif
#ifdef PLAT_PMC
    VK_F4,              //SW5
#else
    VK_APP4,            //SW5 // Scan Code 0x93
#endif
    VK_MENU,            // Scan Code 0x94
    VK_UP,              // Scan Code 0x95
    VK_LEFT,            // Scan Code 0x96
    VK_RIGHT,           // Scan Code 0x97
    VK_DOWN,            // Scan Code 0x98
    VK_LWIN,            // Scan Code 0x99
};

static UINT8 E0ScanCodeToVKeyTable[] =
{
    VK_BROWSER_SEARCH,  // Scan Code 0xE010
    VK_RMENU,           // Scan Code 0xE011
    0,                  // Scan Code 0xE012 which must be 0 or extra WM_KEYDOWN/UP messages will occur
    0,                  // Scan Code 0xE013
    VK_RCONTROL,        // Scan Code 0xE014
    VK_MEDIA_PREV_TRACK,// Scan Code 0xE015
    0,                  // Scan Code 0xE016
    0,                  // Scan Code 0xE017
    VK_BROWSER_FAVORITES,// Scan Code 0xE018
    0,                  // Scan Code 0xE019
    0,                  // Scan Code 0xE01A
    0,                  // Scan Code 0xE01B
    0,                  // Scan Code 0xE01C
    0,                  // Scan Code 0xE01D
    0,                  // Scan Code 0xE01E
    VK_LWIN,            // Scan Code 0xE01F
    VK_BROWSER_REFRESH, // Scan Code 0xE020
    VK_VOLUME_DOWN,     // Scan Code 0xE021
    0,                  // Scan Code 0xE022
    VK_VOLUME_MUTE,     // Scan Code 0xE023
    0,                  // Scan Code 0xE024
    0,                  // Scan Code 0xE025
    0,                  // Scan Code 0xE026
    VK_RWIN,            // Scan Code 0xE027
    VK_BROWSER_STOP,    // Scan Code 0xE028
    0,                  // Scan Code 0xE029
    0,                  // Scan Code 0xE02A
    VK_LAUNCH_APP2,     // Scan Code 0xE02B
    0,                  // Scan Code 0xE02C
    0,                  // Scan Code 0xE02D
    0,                  // Scan Code 0xE02E
    VK_APPS,            // Scan Code 0xE02F
    VK_BROWSER_FORWARD, // Scan Code 0xE030
    0,                  // Scan Code 0xE031
    VK_VOLUME_UP,       // Scan Code 0xE032
    0,                  // Scan Code 0xE033
    VK_MEDIA_PLAY_PAUSE,// Scan Code 0xE034
    0,                  // Scan Code 0xE035
    0,                  // Scan Code 0xE036
    0,                  // Scan Code 0xE037
    VK_BROWSER_BACK,    // Scan Code 0xE038
    0,                  // Scan Code 0xE039
    VK_BROWSER_HOME,    // Scan Code 0xE03A
    VK_MEDIA_STOP,      // Scan Code 0xE03B
    0,                  // Scan Code 0xE03C
    0,                  // Scan Code 0xE03D
    0,                  // Scan Code 0xE03E
    VK_OFF,             // Scan Code 0xE03F
    VK_LAUNCH_APP1,     // Scan Code 0xE040
    0,                  // Scan Code 0xE041
    0,                  // Scan Code 0xE042
    0,                  // Scan Code 0xE043
    0,                  // Scan Code 0xE044
    0,                  // Scan Code 0xE045
    0,                  // Scan Code 0xE046
    0,                  // Scan Code 0xE047
    VK_LAUNCH_MAIL,     // Scan Code 0xE048
    0,                  // Scan Code 0xE049
    VK_DIVIDE,          // Scan Code 0xE04A
    0,                  // Scan Code 0xE04B
    0,                  // Scan Code 0xE04C
    VK_MEDIA_NEXT_TRACK,// Scan Code 0xE04D
    0,                  // Scan Code 0xE04E
    0,                  // Scan Code 0xE04F
    VK_LAUNCH_MEDIA_SELECT,// Scan Code 0xE050
    0,                  // Scan Code 0xE051
    0,                  // Scan Code 0xE052
    0,                  // Scan Code 0xE053
    0,                  // Scan Code 0xE054
    0,                  // Scan Code 0xE055
    0,                  // Scan Code 0xE056
    0,                  // Scan Code 0xE057
    0,                  // Scan Code 0xE058
    0,                  // Scan Code 0xE059
    VK_RETURN,          // Scan Code 0xE05A
    0,                  // Scan Code 0xE05B
    0,                  // Scan Code 0xE05C
    0,                  // Scan Code 0xE05D
    0,                  // Scan Code 0xE05E
    0,                  // Scan Code 0xE05F
    0,                  // Scan Code 0xE060
    0,                  // Scan Code 0xE061
    0,                  // Scan Code 0xE062
    0,                  // Scan Code 0xE063
    0,                  // Scan Code 0xE064
    0,                  // Scan Code 0xE065
    0,                  // Scan Code 0xE066
    0,                  // Scan Code 0xE067
    0,                  // Scan Code 0xE068
    VK_END,             // Scan Code 0xE069
    0,                  // Scan Code 0xE06A
    VK_LEFT,            // Scan Code 0xE06B
    VK_HOME,            // Scan Code 0xE06C
    0,                  // Scan Code 0xE06D
    0,                  // Scan Code 0xE06E
    0,                  // Scan Code 0xE06F
    VK_INSERT,          // Scan Code 0xE070
    VK_DELETE,          // Scan Code 0xE071
    VK_DOWN,            // Scan Code 0xE072
    0,                  // Scan Code 0xE073
    VK_RIGHT,           // Scan Code 0xE074
    VK_UP,              // Scan Code 0xE075
    0,                  // Scan Code 0xE076
    0,                  // Scan Code 0xE077
    0,                  // Scan Code 0xE078
    0,                  // Scan Code 0xE079
    VK_NEXT,            // Scan Code 0xE07A
    0,                  // Scan Code 0xE07B
    VK_SNAPSHOT,        // Scan Code 0xE07C
    VK_PRIOR,           // Scan Code 0xE07D
    VK_CANCEL,          // Scan Code 0xE07E
    0,                  // Scan Code 0xE07F
};

static ScanCodeToVKeyData scvkEngUS =
{
    0,
    ScanCodeTableFirst,
    ScanCodeTableLast,
    ScanCodeToVKeyTable
};

static ScanCodeToVKeyData scvkE0EngUS =
{
    0xE000,
    E0ScanCodeTableFirst,
    E0ScanCodeTableLast,
    E0ScanCodeToVKeyTable
};

static ScanCodeToVKeyData *rgscvkKPPEngUSTables[] =
{
    &scvkEngUS, &scvkE0EngUS
};

// There is currently no Key Remapping being performed.
// If key remapping is needed, this structure definition
// will need to be moved down below the Remap function.
static DEVICE_LAYOUT dlKPPEngUs =
{
    sizeof(DEVICE_LAYOUT),
    KPP_PDD,
    rgscvkKPPEngUSTables,
    dim(rgscvkKPPEngUSTables),
    NULL, //KPPUsRemapVKey,
};

//------------------------------------------------------------------------------
// Local Functions
BOOL BSPKppSetClockGatingMode(BOOL startClocks);
static UINT KppScanSequence(UINT32 rguiScanCode[16], BOOL rgfKeyUp[16], BOOL allowSysCalls);
static void KppRestoreRegState();
static void KppSaveRegState();
static void KppBusyWait(DWORD waitTime);


//------------------------------------------------------------------------------
//
// Function: KPPLayout
//
// This function initializes the device layout for the EVB keypad.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
extern "C" BOOL KPPLayout(PDEVICE_LAYOUT pDeviceLayout)
{
    BOOL fRet = FALSE;

    DEBUGCHK(pDeviceLayout != NULL);

    if (pDeviceLayout->dwSize != sizeof(DEVICE_LAYOUT))
    {
        RETAILMSG(1, (_T("KPPLayout: data structure size mismatch\r\n")));
        goto Leave;
    }

    // Make sure that the Sc->Vk tables are the sizes that we expect
    DEBUGCHK(dim(ScanCodeToVKeyTable) == (1 + ScanCodeTableLast - ScanCodeTableFirst));

    *pDeviceLayout = dlKPPEngUs;

    fRet = TRUE;

Leave:
    return fRet;
}

#ifdef DEBUG
// Verify function declaration against the typedef.
static PFN_DEVICE_LAYOUT_ENTRY v_pfnDeviceLayout = KPPLayout;
#endif


//------------------------------------------------------------------------------
//
// Function: BSPKppRegInit
//
// Initializes the keypad port registers.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE or FALSE.
//
//------------------------------------------------------------------------------
BOOL BSPKppRegInit()
{
    PHYSICAL_ADDRESS phyAddr;
    KPP_FUNCTION_ENTRY();

    phyAddr.QuadPart = CSPKppGetBaseRegAddr();

    //map KPP memory space
    if (g_pKPP == NULL)
        g_pKPP = (PCSP_KPP_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_KPP_REGS), FALSE);
    if(g_pKPP == NULL)
    {
        RETAILMSG(1, (TEXT("%s: KPP memory space mapping failed!\r\n"),
                      __WFUNCTION__));
        return FALSE;
    }
    DEBUGMSG(ZONE_INIT,
             (TEXT("[KPP]%s:  g_pKPP=0x%x\r\n"),
             __WFUNCTION__, g_pKPP));

    // Configure IOMUX to request KPP pins
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL0, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL1, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL2, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_COL3, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW0, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW1, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW2, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    DDKIomuxSetPinMux(DDK_IOMUX_PIN_KPP_ROW3, DDK_IOMUX_PIN_MUXMODE_ALT0,DDK_IOMUX_PIN_SION_REGULAR);
    
    // CONSTANT SETTINGS:   
    // low/high output voltage to NA
    // test_ts to Disabled
    // dse test to regular
    // strength mode to 4_level
    // DDR / CMOS Input Mode to NA
    // Open Drain Enable to Disabled
    // CONFIGURED SETTINGS:
    // Hyst. Enable to Enabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull / Keep Select to Pull
    // Pull Up / Down Config. to CFG(100Kohm PU)
    // Drive Strength to CFG(High)
    // Slew Rate to CFG(FAST)
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW0,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_UP_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE,
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW1,
                         DDK_IOMUX_PAD_SLEW_SLOW,
                         DDK_IOMUX_PAD_DRIVE_NORMAL,
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE,
                         DDK_IOMUX_PAD_PULL_UP_100K,
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW2,
                         DDK_IOMUX_PAD_SLEW_SLOW, 
                         DDK_IOMUX_PAD_DRIVE_NORMAL, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_ROW3, 
                         DDK_IOMUX_PAD_SLEW_SLOW, 
                         DDK_IOMUX_PAD_DRIVE_NORMAL, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);

    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_COL0, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_COL1, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_COL2, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);
    DDKIomuxSetPadConfig(DDK_IOMUX_PAD_KPP_COL3, 
                         DDK_IOMUX_PAD_SLEW_FAST, 
                         DDK_IOMUX_PAD_DRIVE_HIGH, 
                         DDK_IOMUX_PAD_OPENDRAIN_DISABLE, 
                         DDK_IOMUX_PAD_PULL_UP_100K, 
                         DDK_IOMUX_PAD_HYSTERESIS_ENABLE, 
                         DDK_IOMUX_PAD_VOLTAGE_3V3);


    // CONSTANT SETTINGS:   
    // low/high output voltage to NA
    // test_ts to Disabled
    // dse test to regular
    // strength mode to 4_level
    // DDR / CMOS Input Mode to NA
    // Pull / Keep Select to Pull
    // CONFIGURED SETTINGS:
    // Hyst. Enable to Enabled
    // Pull / Keep Enable to CFG(Enabled)
    // Pull Up / Down Config. to CFG(100Kohm PU)
    // Open Drain Enable to CFG(Disabled)
    // Drive Strength to CFG(High)
    // Slew Rate to CFG(FAST)
    //reg32_write(IOMUXC_SW_PAD_CTL_PAD_KEY_COL3, 0x01a5);



    // Enable KPP clocks to access KPP registers
    BSPKppSetClockGatingMode(TRUE);

    // Enable no. of rows in keypad (KRE = 1)
    // Configure columns as open-drain (KCO = 1)
    INSREG16(&g_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KRE),
        CSP_BITFVAL(KPP_KPCR_KRE, KPP_ROW_MASK));
    INSREG16(&g_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KCO),
        CSP_BITFVAL(KPP_KPCR_KCO, KPP_COLUMN_MASK));

    // Write 0's to all columns
    INSREG16(&g_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KCD),
        CSP_BITFVAL(KPP_KPDR_KCD, 0));

    // Configure rows as input, columns as output
    INSREG16(&g_pKPP->KDDR, CSP_BITFMASK(KPP_KDDR_KCDD),
        CSP_BITFVAL(KPP_KDDR_KCDD, KPP_COLUMN_MASK));
    INSREG16(&g_pKPP->KDDR, CSP_BITFMASK(KPP_KDDR_KRDD),
        CSP_BITFVAL(KPP_KDDR_KRDD, 0));

    // Clear KPKD and KPSR_KPKR status flag (w1c)
    // Clear synchronizer chain - KDSC (w1c)
    // Enable keypad interrupt - Set KDIE,
    // clear KRIE (avoid false release events)
    OUTREG16(&g_pKPP->KPSR,
        (CSP_BITFVAL(KPP_KPSR_KPP_EN, KPP_KPSR_KPP_EN_ENABLE) |
        CSP_BITFVAL(KPP_KPSR_KPKD, KPP_KPSR_KPKD_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KPKR, KPP_KPSR_KPKR_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KDSC, KPP_KPSR_KDSC_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KDIE, KPP_KPSR_KDIE_INT_ENABLE) |
        CSP_BITFVAL(KPP_KPSR_KRIE, KPP_KPSR_KRIE_INT_DISABLE)));

    DEBUGMSG(ZONE_PDD, (TEXT("End of Init method - ctrl: %x  status: %x  direction: %x data: %x\r\n"),
                    g_pKPP->KPCR, g_pKPP->KPSR, g_pKPP->KDDR, g_pKPP->KPDR));

    // Disable KPP clocks for Power Management
    BSPKppSetClockGatingMode(FALSE);

    KPP_FUNCTION_EXIT();
    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: BSPKPPGetScanCodes
//
// This function gets the scan codes and key events from the KPP.
//
// Parameters:
//      rguiScanCode[16] -
//          [out] An array of scan codes for each key event detected.
//
//      rgfKeyUp[16] -
//          [out] An array of booleans telling, for each key event,
//          whether the key has gone up or down.
//
// Returns:
//      The number of key events.
//
//------------------------------------------------------------------------------
UINT BSPKPPGetScanCodes(UINT32 rguiScanCode[16], BOOL rgfKeyUp[16])
{
    UINT eventCount;

    KPP_FUNCTION_ENTRY();

    // Enable KPP clocks to access registers
    // during key scan sequence
    BSPKppSetClockGatingMode(TRUE);

    KppSaveRegState();

    eventCount = KppScanSequence(rguiScanCode, rgfKeyUp, TRUE);

    if (g_SysSuspend)
    {
        KppRestoreRegState();

        g_SysSuspend = FALSE;

        // Call scan sequence again with restored KPP register
        // state to assure that we read the correct scan codes.
        eventCount = KppScanSequence(rguiScanCode, rgfKeyUp, TRUE);
    }

    // Disable KPP clocks for Power Management
    BSPKppSetClockGatingMode(FALSE);

    // Wait, so that we do not immediately trigger another keypad
    // interrupt upon completing the scan sequence and returning
    // the scan codes.
    Sleep(9);

    KPP_FUNCTION_EXIT();

    return eventCount;
}


//------------------------------------------------------------------------------
//
// Function: KppScanSequence
//
// This function scans the keypad matrix, compiling a list of key events.
//
// Parameters:
//      rguiScanCode[16] -
//          [out] An array of scan codes for each key event detected.
//
//      rgfKeyUp[16] -
//          [out] An array of booleans telling, for each key event,
//          whether the key has gone up or down.
//
//      allowSysCalls -
//          [in] A boolean variable telling us whether or not we can
//          make system calls.  This is needed for the case when we are
//          entering suspend and cannot make system calls.
//
// Returns:
//      The number of key events.
//
//------------------------------------------------------------------------------
static UINT KppScanSequence(UINT32 rguiScanCode[16], BOOL rgfKeyUp[16], BOOL allowSysCalls)
{
    static BOOL notInitialized = TRUE;  // Set to false after initialization.
    static CRITICAL_SECTION g_hKppLock;
    UINT16 tempKPSR; // KPSR value read at start of scan sequence    
    UINT8 iRowMask;
    UINT8 iColMask;
    UINT8 iCol;
    UINT8 iRow;
    UINT8 evCnt = 0;
    UINT8 index;
    UINT8 rowData;
    BOOL  isKeyDown = FALSE;

    KPP_FUNCTION_ENTRY();

    // Initialize variables the first time this method is called
    if (notInitialized)
    {
        // Do not execute initialization again
        notInitialized = FALSE;

        // create KPP critical section
        InitializeCriticalSection(&g_hKppLock);

        // Initialise key status to all release '1' and clear key down status.
        memset(kppStatus, KPP_ROW_MASK, sizeof(kppStatus));
        memset(keyDown, 0, sizeof(keyDown));
        memset(prevDownTime, 0, sizeof(prevDownTime));
        memset(keyDownSent, 0, sizeof(keyDownSent));
    }

    if (allowSysCalls)
    {
        EnterCriticalSection(&g_hKppLock);
    }

    // Read keypad status register
    tempKPSR = INREG16(&g_pKPP->KPSR);

    DEBUGMSG(ZONE_PDD, (TEXT("Before scan, tempKPSR 0x%04x, KPSR 0x%04x. \r\n"), 
        tempKPSR, INREG16(&g_pKPP->KPSR)));

    // Disable interrupts while processing.
    INSREG16(&g_pKPP->KPSR, CSP_BITFMASK(KPP_KPSR_KDIE),
        CSP_BITFVAL(KPP_KPSR_KDIE, KPP_KPSR_KDIE_INT_DISABLE));
    INSREG16(&g_pKPP->KPSR, CSP_BITFMASK(KPP_KPSR_KRIE),
        CSP_BITFVAL(KPP_KPSR_KRIE, KPP_KPSR_KRIE_INT_DISABLE));

    if(CSP_BITFEXT(tempKPSR, KPP_KPSR_KPKD) && CSP_BITFEXT(tempKPSR, KPP_KPSR_KDIE))
    {
        // At least 1 key depressed
        DEBUGMSG(ZONE_PDD, 
            (TEXT("Depress interrupt, KPSR 0x%04x. \r\n"), tempKPSR));

        // Write '1' to all columns
        INSREG16(&g_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KCD),
            CSP_BITFVAL(KPP_KPDR_KCD, KPP_COLUMN_MASK));

        // Configure column as totem-pole outputs
        INSREG16(&g_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KCO),
            CSP_BITFVAL(KPP_KPCR_KCO, ~KPP_COLUMN_MASK));

        // Configure columns as open drain
        INSREG16(&g_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KCO),
            CSP_BITFVAL(KPP_KPCR_KCO, KPP_COLUMN_MASK));

        // Scan key map for changes
        for(iCol = 0, iColMask = 1; iCol < KPP_COLUMN_INUSE  && !g_SysSuspend; iCol++, iColMask <<= 1)
        {
            // Write '0' for this column.
            INSREG16(&g_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KCD),
                CSP_BITFVAL(KPP_KPDR_KCD, ~iColMask));
            // Wait required to allow row outputs to propagate
            if (allowSysCalls)
            {
                Sleep(1);
            }
            else
            {
                KppBusyWait(1);
            }

            // Get current key status & handle accordingly
            rowData = (UINT8) (KPP_ROW_MASK & EXTREG16(&g_pKPP->KPDR,
                CSP_BITFMASK(KPP_KPDR_KRD), KPP_KPDR_KRD_LSH));

            for(iRow = 0, iRowMask = 1; iRow < KPP_ROW_INUSE && !g_SysSuspend; iRow++, iRowMask <<= 1)
            {
                if((rowData & iRowMask) ^ (kppStatus[iCol] & iRowMask))
                {
                    // Key status changed. Send event accordingly.
                    index = iCol * KPP_ROW_INUSE + iRow;

                    if((rowData & iRowMask))
                    {
                        // Key status changed to released.
                        // Handle briefly pressed keys.
                        if(!(kppStatus[iCol] & iRowMask) && !(keyDown[iCol] & iRowMask))
                        {
                            // Key depressed very briefly, less than 1 debounce period.
                            DEBUGMSG(ZONE_PDD, (TEXT("Changed: depressed < 1 period.\r\n")));
                        }
                        else 
                        {
                            rguiScanCode[evCnt] = IntermediateScanCode[index];
                            rgfKeyUp[evCnt] = TRUE;
                            evCnt++;
                            keyDown[iCol] &= ~iRowMask;
                            keyDownSent[index] = FALSE;

                            DEBUGMSG(ZONE_PDD, 
                                (TEXT("Key released, tempKPSR 0x%04x, KPSR 0x%04x. \r\n"),
                                tempKPSR, INREG16(&g_pKPP->KPSR)));
                        }
                    }
                    else
                    {
                        // Key status changed to depressed.
                        isKeyDown = TRUE;
                        DEBUGMSG(ZONE_PDD, 
                            (TEXT("Key pressed, tempKPSR 0x%04x, KPSR 0x%04x. \r\n"), 
                            tempKPSR, INREG16(&g_pKPP->KPSR)));
                        prevDownTime[index] = GetTickCount();
                    }
                }
                else // No key status change
                {
                    if(!(rowData & iRowMask))
                    {
                        // Key still depressed.
                        // Send key down event after debouncing period.
                        isKeyDown = TRUE;
                        index = iCol * KPP_ROW_INUSE + iRow;

                        if(GetTickCount() < prevDownTime[index])
                        {
                            prevDownTime[index] = 0;
                        }
                // *2 changed by loren to avoid glitch
                        if((GetTickCount() - prevDownTime[index]) >= KEY_DEBOUNCE_PERIOD *2)
                        {

                            if(!(keyDown[iCol] & iRowMask))
                            {
                                keyDown[iCol] |= iRowMask;
                            }

                            DEBUGMSG(ZONE_PDD, 
                                (TEXT("Keypress debounced. \r\n")));
                            if (!keyDownSent[index]) 
                            {
                                // Key down not yet sent, so process
                                rguiScanCode[evCnt] = IntermediateScanCode[index];
                                rgfKeyUp[evCnt] = FALSE;
                                evCnt++;
                                keyDownSent[index] = TRUE;
                            }
                        }
                    }
                }
            }
            // Store current keypad status
            kppStatus[iCol] = rowData;
        }

        // Done keypad scanning.
        INSREG16(&g_pKPP->KPDR, CSP_BITFMASK(KPP_KPDR_KCD), 
            CSP_BITFVAL(KPP_KPDR_KCD, ~KPP_COLUMN_MASK));
        INSREG16(&g_pKPP->KPCR, CSP_BITFMASK(KPP_KPCR_KCO), 
            CSP_BITFVAL(KPP_KPCR_KCO, KPP_COLUMN_MASK));

        // Clear KPKD and KPKR status bits by writing a 1.
        // Set the KPKR synchronizer chain by writing a 1 to KRSS.
        // Clear the KPKD synchronizer chain by writing a 1 to KDSC.
        // Re-enable KDIE and KRIE to detect key hold and key release events.
        OUTREG16(&g_pKPP->KPSR, CSP_BITFVAL(KPP_KPSR_KPP_EN, KPP_KPSR_KPP_EN_ENABLE) |
            CSP_BITFVAL(KPP_KPSR_KRSS, KPP_KPSR_KRSS_SET) |
            CSP_BITFVAL(KPP_KPSR_KPKR, KPP_KPSR_KPKR_CLEAR) |
            CSP_BITFVAL(KPP_KPSR_KPKD, KPP_KPSR_KPKD_CLEAR) |
            CSP_BITFVAL(KPP_KPSR_KDSC, KPP_KPSR_KDSC_CLEAR) |
            CSP_BITFVAL(KPP_KPSR_KDIE, KPP_KPSR_KDIE_INT_ENABLE) |
            CSP_BITFVAL(KPP_KPSR_KRIE, KPP_KPSR_KRIE_INT_ENABLE));
    }
    else if(CSP_BITFEXT(tempKPSR, KPP_KPSR_KPKR) && CSP_BITFEXT(tempKPSR, KPP_KPSR_KRIE))
    {
    // At least 1 key released
        DEBUGMSG(ZONE_PDD, 
            (TEXT("Release interrupt, KPSR 0x%04x. \r\n"), tempKPSR));

        // All configured keys released. Reset all key indicators
        // and send key up event for all keys marked as down.
        for(iCol = 0; iCol < KPP_COLUMN_INUSE && !g_SysSuspend; iCol++)
        {
            for(iRow = 0, iRowMask = 1; iRow < KPP_ROW_INUSE && !g_SysSuspend; iRow++, iRowMask <<= 1)
            {
                index = iCol * KPP_ROW_INUSE + iRow;

                if(keyDown[iCol] & iRowMask)
                {
                    // Handle keys marked as down.
                    rguiScanCode[evCnt] = IntermediateScanCode[index];
                    rgfKeyUp[evCnt] = TRUE;
                    evCnt++;
                    keyDownSent[index] = FALSE;
                }
                else
                {
                    // Take care of keys that are 
                    // depressed only very briefly.
                    if(!(kppStatus[iCol] & iRowMask))
                    {
                        // Key depressed very briefly, less 
                        // than 1 debounce period.
                        DEBUGMSG(ZONE_PDD, 
                            (TEXT("Keys depressed < 1 period.\r\n")));
                    }
                }
            }
            // Clear indicators.
            keyDown[iCol] = (UINT8) ~KPP_ROW_MASK;
            kppStatus[iCol] = KPP_ROW_MASK;
        }
        // Disable key release interrupts and re-enable key depress interrupts.
        OUTREG16(&g_pKPP->KPSR,
            (CSP_BITFVAL(KPP_KPSR_KPP_EN, KPP_KPSR_KPP_EN_ENABLE) |
            CSP_BITFVAL(KPP_KPSR_KDIE, KPP_KPSR_KDIE_INT_ENABLE) |
            CSP_BITFVAL(KPP_KPSR_KRIE, KPP_KPSR_KRIE_INT_DISABLE) |
            CSP_BITFVAL(KPP_KPSR_KPKR, KPP_KPSR_KPKR_CLEAR) |
            CSP_BITFVAL(KPP_KPSR_KPKD, KPP_KPSR_KPKD_CLEAR) |
            CSP_BITFVAL(KPP_KPSR_KDSC, KPP_KPSR_KDSC_CLEAR)));
    }
    else
    {
        OUTREG16(&g_pKPP->KPSR,
            (CSP_BITFVAL(KPP_KPSR_KPP_EN, KPP_KPSR_KPP_EN_ENABLE) |
            CSP_BITFVAL(KPP_KPSR_KDIE, KPP_KPSR_KDIE_INT_ENABLE) |
            CSP_BITFVAL(KPP_KPSR_KRIE, KPP_KPSR_KRIE_INT_DISABLE) |
            CSP_BITFVAL(KPP_KPSR_KPKR, KPP_KPSR_KPKR_CLEAR) |
            CSP_BITFVAL(KPP_KPSR_KPKD, KPP_KPSR_KPKD_CLEAR) |
            CSP_BITFVAL(KPP_KPSR_KDSC, KPP_KPSR_KDSC_CLEAR)));
    }
    DEBUGMSG(ZONE_PDD, (TEXT("Scan sequence completed\r\n")));

    if (allowSysCalls)
    {
        LeaveCriticalSection(&g_hKppLock);
    }

    if (evCnt > MAX_KEY_EVENTS)
    {
        evCnt = MAX_KEY_EVENTS;
    }

    KPP_FUNCTION_EXIT();

    return evCnt;
}


//------------------------------------------------------------------------------
//
// Function: BSPKppPowerOn
//
// Power on the keypad.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL BSPKppPowerOn()
{
    UINT16 kpsr;

    KPP_FUNCTION_ENTRY();

    BSPKppRegInit();

    // Enable KPP clocks to access KPP registers
    BSPKppSetClockGatingMode(TRUE);

    // Until keypad does not report key-down/key-up event
    do
    {        
        // Wait ~10ms
        KppBusyWait(10);
                
        // Get current status
        kpsr = INREG16(&g_pKPP->KPSR);

        // Clear status bits
        OUTREG16(&g_pKPP->KPSR, (kpsr | CSP_BITFMASK(KPP_KPSR_KPKD)));
        
    }while (kpsr & CSP_BITFMASK(KPP_KPSR_KPKD));

    if (!g_bRestoreClocks)
    {
        // Disable KPP clocks for Power Management
        BSPKppSetClockGatingMode(FALSE);
    }

    KPP_FUNCTION_EXIT();
    return(TRUE);
}


//------------------------------------------------------------------------------
//
// Function: BSPKppPowerOff
//
// Power off the keypad.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL BSPKppPowerOff()
{
    UINT16 kpsr;
    DDK_CLOCK_GATE_MODE lastKPPClockMode;
    
    KPP_FUNCTION_ENTRY();
    
    // Tell the IST we are entering suspend state
    g_SysSuspend = TRUE;


    // Get current KPP clock mode
    DDKClockGetGatingMode(DDK_CLOCK_GATE_INDEX_KPP, &lastKPPClockMode);
 
    if (lastKPPClockMode == DDK_CLOCK_GATE_MODE_ENABLED)
    {
        g_bRestoreClocks = TRUE;
    }
    else
    {
        g_bRestoreClocks = FALSE;
    }

    BSPKppRegInit();   // Clears KPKD and KPKR

    // Enable KPP clocks to access KPP registers
    BSPKppSetClockGatingMode(TRUE);

    // Until keypad does not report key-down/key-up event
    do
    {        
        // Wait ~10ms
        KppBusyWait(10);
                
        // Get current status
        kpsr = INREG16(&g_pKPP->KPSR);

        // Clear status bits
        OUTREG16(&g_pKPP->KPSR, (kpsr | CSP_BITFMASK(KPP_KPSR_KPKD)));
        
    }while (kpsr & CSP_BITFMASK(KPP_KPSR_KPKD));

    // Disable KPP clocks for Power Management
    BSPKppSetClockGatingMode(FALSE);

    KPP_FUNCTION_EXIT();
    return(TRUE);
}

//------------------------------------------------------------------------------
//
// Function: BSPKppSetClockGatingMode
//
// Turn on/off clocks to the keypad port module.
//
// Parameters:
//      startClocks
//          [in] If TRUE, turn clocks to KPP on.
//                If FALSE, turn clocks to KPP off
//
// Returns:
//      TRUE if success, FALSE if failure.
//
//------------------------------------------------------------------------------
BOOL BSPKppSetClockGatingMode(BOOL startClocks)
{

    if (startClocks)
    {
        // Turn KPP clocks on
        if (!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_KPP, 
            DDK_CLOCK_GATE_MODE_ENABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed to set CRM clock gating mode!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }
    else
    {
        // Turn KPP clocks off
        if (!DDKClockSetGatingMode(DDK_CLOCK_GATE_INDEX_KPP, 
            DDK_CLOCK_GATE_MODE_DISABLED))
        {
            DEBUGMSG(ZONE_ERROR, (TEXT("%s: Failed to set CRM clock gating mode!\r\n"), __WFUNCTION__));
            return FALSE;
        }
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
// Function: BSPKppIsWakeUpSource
//
// Defines if the Keypad should wake the system up from a suspend state.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if the keypad must be considered as a wake-up source, FALSE otherwise
//
//------------------------------------------------------------------------------
BOOL BSPKppIsWakeUpSource()
{
    return FALSE;
}

//------------------------------------------------------------------------------
//
// Function: KppSaveRegState
//
// Saves the state of KPP registers, in case the system changes to
// the suspend state during the key scan sequence.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
static void KppSaveRegState()
{
    g_wKPSRStored = INREG16(&g_pKPP->KPSR);
    memcpy(keyDownStored, keyDown, KPP_COLUMN_INUSE);
    memcpy(prevDownTimeStored, prevDownTime, sizeof(prevDownTimeStored));
    memcpy(keyDownSentStored, keyDownSent, sizeof(keyDownSentStored));
    memcpy(kppStatusStored, kppStatus, KPP_COLUMN_INUSE);
}

//------------------------------------------------------------------------------
//
// Function: KppRestoreRegState
//
// Restores the state of KPP registers, for when the system returns from
// the suspend state during the key scan sequence.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
static void KppRestoreRegState()
{
    memcpy(keyDown, keyDownStored, KPP_COLUMN_INUSE);
    memcpy(prevDownTime, prevDownTimeStored, KEY_NUMBER * sizeof(UINT32));
    memcpy(keyDownSent, keyDownSentStored, KEY_NUMBER * sizeof(BOOL));
    memcpy(kppStatus, kppStatusStored, KPP_COLUMN_INUSE);

    // Restore interrupts to their pre-suspend state, 
    // and clear status registers.
    OUTREG16(&g_pKPP->KPSR, 
        (CSP_BITFVAL(KPP_KPSR_KPP_EN, CSP_BITFEXT(g_wKPSRStored, KPP_KPSR_KPP_EN)) |
        CSP_BITFVAL(KPP_KPSR_KDIE, CSP_BITFEXT(g_wKPSRStored, KPP_KPSR_KDIE)) |
        CSP_BITFVAL(KPP_KPSR_KRIE, CSP_BITFEXT(g_wKPSRStored, KPP_KPSR_KRIE)) |
        CSP_BITFVAL(KPP_KPSR_KPKD, KPP_KPSR_KPKD_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KPKR, KPP_KPSR_KPKR_CLEAR) |
        CSP_BITFVAL(KPP_KPSR_KRSS, KPP_KPSR_KRSS_SET) |
        CSP_BITFVAL(KPP_KPSR_KDSC, KPP_KPSR_KDSC_CLEAR)));

    // Wait 1 ms for sychronizer chain to clear
    Sleep(1);
}

//------------------------------------------------------------------------------
//
// Function: KppBusyWait
//
// Uses GetTickCount to wait.
//
// Parameters:
//      waitTime
//          [in] Time to wait in milliseconds.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
static void KppBusyWait(DWORD waitTime)
{
    DWORD startTime, currentTime, realWaitTime;

    realWaitTime = waitTime + 1;

    // Wait for 10 ms, then check KPKD again
    startTime = GetTickCount();
    do
    {
        currentTime = GetTickCount();
        if (currentTime < startTime)
        {
            startTime = currentTime;
        }
    } while(currentTime - startTime < realWaitTime);
}
