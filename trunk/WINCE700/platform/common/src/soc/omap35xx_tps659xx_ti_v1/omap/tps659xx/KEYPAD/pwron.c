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
//  File: pwron.c
//
//  Handles the PWRON signal of T2 and translates it into a keypress.
//
#include <windows.h>
#include <winuser.h>
#include <winuserm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <tps659xx.h>
#include <keypad.h>
#include <pmpolicy.h>

#include <twl.h>
#include "_keypad.h"

//------------------------------------------------------------------------------
//
//  Function:  PhysicalStateToVirtualState
//
//  Convert physical state to virtual keys state
//
static VOID
PhysicalStateToVirtualState(
    BOOL bState,
    DWORD vkNewState[],
    BOOL *pKeyDown
    )
{
    BOOL keyDown = FALSE;
    UINT8 vk;

    if (bState != 0)
        {
        // g_keypadVK is defined by the platform
        vk = g_powerVK[0];
        vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
        keyDown = TRUE;
        }
    
    if (keyDown && (pKeyDown != NULL)) *pKeyDown = TRUE;    
}


//------------------------------------------------------------------------------
//
//  Function:  VirtualKeyRemap
//
//  This function remaps multiple virtual key to final virtual key.
//
static VOID
VirtualKeyRemap(
    DWORD time,
    BOOL *pKeyDown,
    KeypadRemapState_t *pRemapState,
    DWORD vkNewState[]
    )
{
    BOOL keyDown = FALSE;
    int ix;
    
    for (ix = 0; ix < g_powerRemap.count; ix++)
        {
        const KEYPAD_REMAP_ITEM *pItem = &g_powerRemap.pItem[ix];
        KeypadRemapState_t *pState = &pRemapState[ix];
        DWORD state = 0;
        USHORT down = 0;
        UINT8 vk = 0;

        // Count number of keys down & save down/up state
        int ik;
        for (ik = 0; ik < pItem->keys; ik++)
            {
            vk = pItem->pVKeys[ik];
            if ((vkNewState[vk >> 5] & (1 << (vk & 0x1F))) != 0)
                {
                state |= 1 << ik;
                down++;
                }
            }

        // Depending on number of keys down
        if (down >= pItem->keys && pItem->keys > 1)
            {
            // Clear all mapping keys
            for (ik = 0; ik < pItem->keys; ik++)
                {
                vk = pItem->pVKeys[ik];
                vkNewState[vk >> 5] &= ~(1 << (vk & 0x1F));
                }
            // All keys are down set final key
            vk = pItem->vkey;
            vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
            DEBUGMSG(ZONE_IST, (L" VirtualKeyRemap: "
                L"Mapped vkey: 0x%x\r\n", vk
                ));
            // Clear remap pending flag
            pState->pending = FALSE;
            // Set remap processing flag
            pState->remapped = TRUE;
            }
        else if ( down > 0 )
            {
            // If already remapping or remapping is not pending
            // or pending time expired
            if  (  pState->remapped ||
                  !pState->pending  ||
                  (INT32)( time - pState->time ) < 0 )
                {
                // If we are not pending and not already remapping, start
                if (!pState->pending && !pState->remapped)
                    {
                    pState->pending = TRUE;
                    pState->time = time + pItem->delay;
                    }
                // Clear all mapping keys
                for (ik = 0; ik < pItem->keys; ik++)
                    {
                    vk = pItem->pVKeys[ik];
                    vkNewState[vk >> 5] &= ~( 1 << ( vk & 0x1F ) );
                    }
                }
            else if (  pItem->keys == 1 &&
                      (INT32)( time - pState->time ) >= 0 )
                {
                // This is press and hold key
               DEBUGMSG(ZONE_IST, (L" VirtualKeyRemap: "
                    L"Mapped press and hold vkey: 0x%x\r\n", vk
                    ));
                vk = pItem->vkey;
                vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                keyDown = TRUE;
                pState->pending = FALSE;
                }
            }
        else
            {
            // All keys are up, if remapping was pending set keys
            if ( pState->pending )
                {
                for (ik = 0; ik < pItem->keys; ik++)
                    {
                    if ( ( pState->state & ( 1 << ik ) ) != 0 )
                        {
                        vk = pItem->pVKeys[ik];
                        vkNewState[vk >> 5] |= 1 << (vk & 0x1F);
                        keyDown = TRUE;
                        }
                    }
                pState->pending = FALSE;
                }
            pState->remapped = FALSE;
            }
        // Save key state
        pState->state = state;
        }

    // Set output variable
    if (keyDown && (pKeyDown != NULL)) *pKeyDown = TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  AutoRepeat
//
//  This function sends KeyEvents for auto-repeat keys.
//
static VOID
AutoRepeat(
    const KeypadDevice_t *pDevice,
    const DWORD vkNewState[],
    DWORD time,
    KeypadRepeatState_t *pRepeatState
    )
{
    ULONG ix;

    for (ix = 0; ix < g_powerRepeat.count; ix++)
        {
        const KEYPAD_REPEAT_ITEM *pItem = &g_powerRepeat.pItem[ix];
        KeypadRepeatState_t *pState = &pRepeatState[ix];
        DWORD delay;
        BOOL blockRepeat = FALSE;
        UINT8 vkBlock;
        UINT8 vk = pItem->vkey;

        if ((vkNewState[vk >> 5] & (1 << (vk & 0x1F))) != 0)
            {
            if (!pState->pending)
                {
                // Key was just pressed
                delay = pItem->firstDelay;
                if (delay == 0) delay = pDevice->firstRepeat;
                pState->time = time + delay;
                pState->pending = TRUE;
                pState->blocked = FALSE;
                }
            else if (((INT32)(time - pState->time)) >= 0)
                {
                // Check if any blocking keys are pressed
                const KEYPAD_REPEAT_BLOCK *pBlock = pItem->pBlock;
                if (pBlock != 0)
                    {
                    int ik;
                    for ( ik = 0; ik < pBlock->count; ik++ )
                        {
                        vkBlock = pBlock->pVKey[ik];
                        if ((vkNewState[vkBlock >> 5] &
                               (1 << (vkBlock & 0x1F))) != 0)
                            {
                            pState->blocked = TRUE;
                            DEBUGMSG(ZONE_IST, (L" AutoRepeat: "
                                L"Block repeat: 0x%x bcause of 0x%x\r\n",
                                vk, vkBlock
                                ));
                            break;
                        }
                    }
                }
                // Repeat if not blocked
                if (!pState->blocked)
                    {
                    DEBUGMSG(ZONE_IST, (L" AutoRepeat: "
                        L"Key Repeat: 0x%x\r\n", vk
                        ));
                    SendKeyPadEvent(vk, 0, pItem->silent ? KEYEVENTF_SILENT : 0, 0);
                    }
                // Set time for next repeat
                delay = pItem->nextDelay;
                if (delay == 0) delay = pDevice->nextRepeat;
                pState->time = time + delay;
                }
            }
        else
            {
            pState->pending = FALSE;
            pState->blocked = FALSE;
            }
        }
}


//-----------------------------------------------------------------------------
//
//  Function:  PWRON_InterruptThread
//
//  This function acts as the IST for the keypad.
//
DWORD PWRON_IntrThread(VOID *pContext)
{
    KeypadDevice_t *pDevice = (KeypadDevice_t*)pContext;
    KeypadRemapState_t *pRemapState = NULL;
    KeypadRepeatState_t *pRepeatState = NULL;
    DWORD vkState[VK_KEYS/DWORD_BITS];
    DWORD vkNewState[VK_KEYS/DWORD_BITS];
    DWORD timeout;
    BOOL matrix = 0;
    BOOL keyDown = FALSE;
    UINT8 val;
    DWORD time;
    DWORD code;
    BOOL bInit = TRUE;

    // Init data
    memset(vkState, 0, sizeof(vkState));
    memset(vkNewState, 0, sizeof(vkNewState));

    // Initialize remap informations
    if (g_powerRemap.count > 0)
        {
        // Allocate state structure for remap, zero initialized
        pRemapState = LocalAlloc(
            LPTR, g_powerRemap.count * sizeof(KeypadRemapState_t)
            );
        if (pRemapState == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L" KPD_IntrThread: "
                L"Failed allocate memory for virtual key remap\r\n"
                ));
            goto cleanUp;
            }
        }

    // Initialize repeat informations
    if (g_powerRepeat.count > 0)
        {
        // Allocate state structure for repeat, zero initialized
        pRepeatState = LocalAlloc(
            LPTR, g_powerRepeat.count * sizeof(KeypadRepeatState_t)
            );
        if (pRepeatState == NULL)
            {
            DEBUGMSG(ZONE_ERROR, (L" KPD_IntrThread: "
                L"Failed allocate memory for virtual key auto repeat\r\n"
                ));
            goto cleanUp;
            }
        }

    // Set delay to sample period
    timeout = INFINITE;

    // Loop until we are not stopped...
    while (!pDevice->intrThreadExit)
        {
        // to make sure we the keypress state of power on/off is
        // in sync with hardware we only enable rising edge 
        // interrupt first and then enable both falling and rising 
        // edge interrupts
        if (bInit == TRUE)
            {
            TWLReadRegs(pDevice->hTWL, TWL_PWR_EDR1, &val, 1);
            val |= TWL_PWRON_RISING;
            TWLWriteRegs(pDevice->hTWL, TWL_PWR_EDR1, &val, 1);
            }
    
        // Wait for event
        code = WaitForSingleObject(pDevice->hIntrEventPower, timeout);
        if (pDevice->intrThreadExit) break;

        // key state
        if (code == WAIT_OBJECT_0)
            {
            // enable both falling and rising edge if not already done
            if (bInit == TRUE)
                {
                TWLReadRegs(pDevice->hTWL, TWL_PWR_EDR1, &val, 1);
                val |= TWL_PWRON_FALLING | TWL_PWRON_RISING;
                TWLWriteRegs(pDevice->hTWL, TWL_PWR_EDR1, &val, 1);
                bInit = FALSE;
                }
            
            matrix ^= 1;
            }

        // Convert physical state to virtual keys state
        keyDown = 0;
        PhysicalStateToVirtualState(matrix, vkNewState, &keyDown);

        time = GetTickCount();

        // Remap multi virtual keys to final virtual key
        VirtualKeyRemap(time, &keyDown, pRemapState, vkNewState);
        PressedReleasedKeys(pDevice, vkState, vkNewState);
        AutoRepeat(pDevice, vkNewState, time, pRepeatState);

        //--------------------------------------------------------------
        // Prepare for next run
        //--------------------------------------------------------------

        // New state become old
        memcpy(vkState, vkNewState, sizeof(vkState));
        // Get new state for virtual key table
        memset(vkNewState, 0, sizeof(vkNewState));

        // Set timeout period depending on data state
        timeout = keyDown ? pDevice->samplePeriod : INFINITE;

        // Interrupt is done
        }

cleanUp:
    return ERROR_SUCCESS;
}

//-----------------------------------------------------------------------------

