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
// Copyright (c) Intrinsyc Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Intrinsyc end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  File:  pdd.c
//
//  This file contains USB function PDD implementation. Actual implementation
//  doesn't use DMA transfers and it doesn't support ISO endpoints.
//
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <omap2420.h>
#include <devload.h>
#include <oal_intr.h>

extern BOOL InitializeHardware();
void PDD_Dump(OMAP2420_USBD_REGS * pPddRegs);
static void printUsbdDriverSettings();


//------------------------------------------------------------------------------
//
//  Define:  USBD_IRQ_MASK
//
//  This is composite interrupt mask used in driver.
//
#define USBD_IRQ_MASK  (USBD_IRQ_EN_EP_RX | USBD_IRQ_EN_EP_TX | USBD_IRQ_EN_DS_CHG | USBD_IRQ_EN_EP0)

//------------------------------------------------------------------------------
//
//  Type:  USBFN_EP
//
//  Internal PDD structure which holds info about endpoint direction,
//  max packet size and active transfer.
//
typedef struct
{
    WORD       maxPacketSize;
    BOOL       dirRx;
    BOOL       fZeroLengthNeeded;
    STransfer *pTransfer;
} USBFN_EP;

//------------------------------------------------------------------------------
//
//  Type:  USBFN_PDD
//
//  Internal PDD context structure.
//
typedef struct
{
    DWORD memBase;
    DWORD memLen;
    DWORD priority256;
    DWORD irq[3];

    VOID *pMddContext;
    PFN_UFN_MDD_NOTIFY pfnNotify;

    HANDLE hParentBus;
    OMAP2420_USBD_REGS *pUSBDRegs;

    DWORD sysIntr;
    HANDLE hIntrEvent;
    BOOL exitIntrThread;
    HANDLE hIntrThread;

    CEDEVICE_POWER_STATE m_NewPowerState;
    CEDEVICE_POWER_STATE m_CurrentPowerState;

    DWORD devState;
    BOOL selfPowered;

    BOOL setupDirRx;
    WORD setupCount;
    HANDLE hSetupEvent;

    CRITICAL_SECTION epCS;
    USBFN_EP ep[USBD_EP_COUNT];

    BOOL fakeDsChange;                  // To workaround MDD problem

} USBFN_PDD;

//------------------------------------------------------------------------------
//  Device registry parameters

static const DEVICE_REGISTRY_PARAM g_deviceRegParams[] = {
    {
        L"MemBase", PARAM_DWORD, TRUE, offset(USBFN_PDD, memBase),
        fieldsize(USBFN_PDD, memBase), NULL
    }, {
        L"MemLen", PARAM_DWORD, TRUE, offset(USBFN_PDD, memLen),
        fieldsize(USBFN_PDD, memLen), NULL
    }, {
        L"Irq", PARAM_MULTIDWORD, TRUE, offset(USBFN_PDD, irq),
        fieldsize(USBFN_PDD, irq), NULL
    }, {
        L"Priority256", PARAM_DWORD, FALSE, offset(USBFN_PDD, priority256),
        fieldsize(USBFN_PDD, priority256), (void *)101
    }
};

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//

#ifdef DEBUG

#define ZONE_INTERRUPTS         DEBUGZONE(8)
#define ZONE_POWER              DEBUGZONE(9)
#define ZONE_PDD                DEBUGZONE(15)

extern DBGPARAM dpCurSettings = {
    L"UsbFn", {
        L"Error",       L"Warning",     L"Init",        L"Transfer",
        L"Pipe",        L"Send",        L"Receive",     L"USB Events",
        L"Interrupts",  L"Power",       L"",            L"",
        L"Function",    L"Comments",    L"",            L"PDD"
    },
    DBG_ERROR|DBG_INIT
};

#endif

//------------------------------------------------------------------------------
//
//  Function:  Log2
//
//  Trivial log with base 2 function used in EP configuration.
//
static DWORD Log2(DWORD value)
{
    DWORD rc = 0;

    while (value != 0)
    {
        value >>= 1;
        rc++;
    }

    if (rc > 0)
    {
        rc--;
    }

    return rc;
}


// Routines to synchronize access to the indexed endpoint registers.
#ifdef DEBUG
static int InEp = 0;
#endif // DEBUG

static VOID Select(USBFN_PDD *pPdd, DWORD dwVal)
{
    EnterCriticalSection(&pPdd->epCS);
    ASSERT(InEp++ >= 0);
    OUTREG32(&pPdd->pUSBDRegs->EP_NUM, dwVal);
}


static VOID SelectSetup(USBFN_PDD *pPdd)
{
    // Select setup FIFO (this clears USBD_INT_SETUP flag)
    Select(pPdd, USBD_EP_NUM_SETUP);
}


static VOID SelectEp(USBFN_PDD *pPdd, DWORD epNum)
{
    Select(pPdd, USBD_EP_NUM_SEL | epNum);
}


static VOID Deselect(USBFN_PDD *pPdd, DWORD dwValue)
{
    OUTREG32(&pPdd->pUSBDRegs->EP_NUM, dwValue);
    ASSERT(--InEp >= 0);
    LeaveCriticalSection(&pPdd->epCS);
}


static VOID DeselectSetup(USBFN_PDD *pPdd)
{
    Deselect(pPdd, 0);
}


static VOID DeselectEp(USBFN_PDD *pPdd, DWORD epNum)
{
    Deselect(pPdd, epNum);
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_PowerDown
//
//  LEGACY FUNCTION: Do not use!
//
//  This function is provided for backwards compatibility only.
//  It is included in the USB Client Driver's function table (which gets
//  exported to the rest of the operating system), and thus other modules
//  may be expecting it to be a callable function.
//
VOID WINAPI UfnPdd_PowerDown(VOID *pPddContext)
{
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_PowerUp
//
//  LEGACY FUNCTION: Do not use!
//
//  This function is provided for backwards compatibility only.
//  It is included in the USB Client Driver's function table (which gets
//  exported to the rest of the operating system), and thus other modules
//  may be expecting it to be a callable function.
//
VOID WINAPI UfnPdd_PowerUp(VOID *pPddContext)
{
}


////////////////////////////////////////////////////////////////////////////////
//
// Function:   UpdateDevicePower()
//
// Purpose:    Puts the USB Client Device in the Power State provided in *pPdd
//
// Parameters: pPdd = Pointer to USBFN_PDD Structure containing device info
//
// Returns:    TRUE if the power state was successfully updated,
//             or FALSE if it was not.
//
// Notes:      Power States D0, D1, and D2 are considered "on".
//             Power States D3 and D4 are considered "off".
//
////////////////////////////////////////////////////////////////////////////////
//
BOOL UpdateDevicePower(USBFN_PDD * pPdd)
{
    BOOL  bRV    = TRUE;
    DWORD regBit = 0x00000001;
    DWORD cbRet  = 0x00000000;


    if (pPdd != NULL)
    {
        DEBUGMSG(ZONE_POWER, (L"USBD UpdateDevicePower() - "
            L"Changing Power State from D%u to D%u\r\n", (int)(pPdd->m_CurrentPowerState), (int)(pPdd->m_NewPowerState)));

        switch (pPdd->m_CurrentPowerState)
        {
            case D0:
            case D1:
            case D2:
                if (SetDevicePowerState(pPdd->hParentBus, pPdd->m_NewPowerState, NULL))
                {
                    if ((pPdd->m_NewPowerState == D3) || (pPdd->m_NewPowerState == D4))
                    {
                        DEBUGMSG(ZONE_POWER, (L"USBD UpdateDevicePower() - "
                            L"Turning the Power Off\r\n"));

                        // Disable the USBD Transceiver
                        if (pPdd->pUSBDRegs)
                        {
                            CLRREG32(&pPdd->pUSBDRegs->SYSCON1, USBD_SYSCON1_PULLUP_EN);
                        }

                        // Turn off the USBD Clocks
                        KernelIoControl(IOCTL_ICLK2_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
                        KernelIoControl(IOCTL_FCLK2_DIS, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
                    }
                }
                else
                {
                    bRV = FALSE;
                }
                break;

            case D3:
            case D4:
                if (SetDevicePowerState(pPdd->hParentBus, pPdd->m_NewPowerState, NULL))
                {
                    if ((pPdd->m_NewPowerState == D0) || (pPdd->m_NewPowerState == D1) || (pPdd->m_NewPowerState == D2))
                    {
                        DEBUGMSG(ZONE_POWER, (L"USBD UpdateDevicePower() - "
                            L"Turning the Power On\r\n"));

                        // Turn on the USBD Clocks
                        KernelIoControl(IOCTL_ICLK2_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);
                        KernelIoControl(IOCTL_FCLK2_ENB, (VOID *)&regBit, sizeof(DWORD), NULL, 0, &cbRet);

                        // Enable the USBD Transceiver
                        if (pPdd->pUSBDRegs)
                        {
                            SETREG32(&pPdd->pUSBDRegs->SYSCON1, USBD_SYSCON1_PULLUP_EN);
                        }
                    }
                }
                else
                {
                    bRV = FALSE;
                }
                break;

            default:
                bRV = FALSE;
                break;
        }

        if (bRV)
        {
            pPdd->m_CurrentPowerState = pPdd->m_NewPowerState;
        }
    }

    return bRV;
}

//------------------------------------------------------------------------------
//
//  Function:  SetupEvent
//
//  This function handles setup packet interrupts.
//
static VOID SetupEvent(USBFN_PDD *pPdd)
{
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD rawFifoData = 0x00000000;
    WORD data[4];
    USB_DEVICE_REQUEST *pSetup = (USB_DEVICE_REQUEST*)data;


    do {
        // Select setup FIFO (this clears USBD_INT_SETUP flag)
        SelectSetup(pPdd);

        // Read setup data
        rawFifoData = INREG32(&pUSBDRegs->DATA);
        data[0]     = (WORD)(rawFifoData & 0x0000FFFF);

        rawFifoData = INREG32(&pUSBDRegs->DATA);
        data[1]     = (WORD)(rawFifoData & 0x0000FFFF);

        rawFifoData = INREG32(&pUSBDRegs->DATA);
        data[2]     = (WORD)(rawFifoData & 0x0000FFFF);

        rawFifoData = INREG32(&pUSBDRegs->DATA);
        data[3]     = (WORD)(rawFifoData & 0x0000FFFF);

        // Deselect setup FIFO
        DeselectSetup(pPdd);
    } while ((INREG32(&pUSBDRegs->IRQ_SRC) & USBD_INT_SETUP) != 0);
/*
    DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - data[0] = 0x%04X\r\n", data[0]));
    DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - data[1] = 0x%04X\r\n", data[1]));
    DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - data[2] = 0x%04X\r\n", data[2]));
    DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - data[3] = 0x%04X\r\n", data[3]));

    // Make sure we are interpreting the above data correctly.
    DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - pSetup->bmRequestType = 0x%02X\r\n", pSetup->bmRequestType));
    DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - pSetup->bRequest      = 0x%02X\r\n", pSetup->bRequest));
    DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - pSetup->wValue        = 0x%04X\r\n", pSetup->wValue));
    DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - pSetup->wIndex        = 0x%04X\r\n", pSetup->wIndex));
    DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - pSetup->wLength       = 0x%04X\r\n", pSetup->wLength));
*/
    // Save setup packet direction & size for later use
    pPdd->setupDirRx = (pSetup->bmRequestType & 0x80) == 0;
    pPdd->setupCount = pSetup->wLength;

    // MDD doesn't call PDD back on configure message
    if ((pSetup->bmRequestType == 0) && (pSetup->bRequest == USB_REQUEST_SET_CONFIGURATION))
    {
        if (pSetup->wValue != 0)
        {
            DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - "
                L"Changing USB Device state to Configured\r\n"));

            // Move device to configured state
            OUTREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_DEV_CFG);

            // Set self powered flag
            if (pPdd->selfPowered)
            {
                DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - "
                    L"USB Device is Self-Powered\r\n"));
                SETREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_SELF_PWR);
            }
            else
            {
                DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - "
                    L"USB Device is NOT Self-Powered\r\n"));
                CLRREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_SELF_PWR);
            }
        }
        else
        {
            DEBUGMSG(ZONE_INIT, (L"USBD SetupEvent() - "
                L"Changing USB Device state to Addressed\r\n"));
            OUTREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_CLR_CFG);
        }
    }

    // Let MDD process message
    pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SETUP_PACKET, (DWORD)data);

}

//------------------------------------------------------------------------------
//
//  Function:  IssueTxTransfer
//
//  This function sends next packet from transaction buffer. It is called from
//  interrupt thread and UfnPdd_IssueTransfer.
//
static VOID IssueTxTransfer(USBFN_PDD *pPdd, DWORD endPoint)
{
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    STransfer *pTransfer;
    BOOL complete = FALSE;
    DWORD stat, space, count, remain;
    UCHAR *pBuffer;
    DWORD data;
    DWORD epNum;

    // Get active transfer
    pTransfer = pPdd->ep[endPoint].pTransfer;

    DEBUGMSG(ZONE_TRANSFER, (L"USBD IssueTxTransfer() - "
        L"EP %d pTransfer 0x%08X (%d, %d, %d)\r\n",
                 endPoint,
                 pTransfer,
                 pTransfer != NULL ? pTransfer->cbBuffer : 0,
                 pTransfer != NULL ? pTransfer->cbTransferred : 0,
                 pTransfer != NULL ? pTransfer->dwUsbError : -1));

    // Select EP
    epNum = (USBD_EP_NUM & endPoint) | USBD_EP_NUM_DIRIN;
    SelectEp(pPdd, epNum);


    // Get EP status
    stat = INREG32(&pUSBDRegs->STAT_FLG);

    // Depending on EP status
    if ((stat & USBD_STAT_STALL) != 0)
    {
        // We issued stall, remove it...
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_HALT);
    }
    else
    {
        // When transfer is NULL it is handshake ACK
        if (pTransfer != NULL)
        {
            // Is this final interrupt of transfer?
            if ((pTransfer->cbTransferred == pTransfer->cbBuffer) && (!pPdd->ep[endPoint].fZeroLengthNeeded))
            {
                pTransfer->dwUsbError = UFN_NO_ERROR;
                complete = TRUE;
            }
            else
            {
                __try
                {
                    pBuffer = (UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
                    space   = pTransfer->cbBuffer - pTransfer->cbTransferred;

                    if (endPoint != 0)
                    {
                        // Non Zero Endpoint: No zero length padding needed.
                        pPdd->ep[endPoint].fZeroLengthNeeded  = FALSE;
                    }
                    else
                    {
                        // Zero endpoint: Zero length padding needed if last packet is maxPacketSize.
                        pPdd->ep[endPoint].fZeroLengthNeeded = ((space == pPdd->ep[endPoint].maxPacketSize) && (pPdd->setupCount > pTransfer->cbBuffer));
                    }

                    // How many bytes we can send just now?
                    count = pPdd->ep[endPoint].maxPacketSize;

                    if (count > space)
                    {
                        count = space;
                    }

                    // Write data to FIFO
                    remain = count;
                    while (remain > 1)
                    {
                        data = (pBuffer[1] << 8) | pBuffer[0];
                        OUTREG16((UINT16*)&pUSBDRegs->DATA, data);

                        pBuffer += 2;
                        space   -= 2;
                        remain  -= 2;
                    }

                    if (remain > 0)
                    {
                        OUTREG8((UINT8*)&pUSBDRegs->DATA, *pBuffer);

                        pBuffer += 1;
                        space   -= 1;
                        remain  -= 1;
                    }

                    DEBUGMSG(ZONE_TRANSFER, (L"USBD IssueTxTransfer() - "
                        L"Transfered %d bytes on ep 0x%02X", count, endPoint));

                    // Enable FIFO
                    OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_FIFO_EN);

                    // We transfered some data
                    pTransfer->cbTransferred = pTransfer->cbBuffer - space;

                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
                    complete = TRUE;
                }
            }
        }
        else
        {
            DEBUGMSG(ZONE_TRANSFER, (L"USBD IssueTxTransfer() - "
                L"pTransfer is NULL; HandShake ACK."));
        }
    }

    // Deselect EP
    DeselectEp(pPdd, epNum);

    // If transaction is complete we should tell MDD
    if (complete)
    {
        DEBUGMSG(ZONE_TRANSFER, (L"USBD IssueTxTransfer() - "
            L"Notifying MDD that transfer is complete."));

        pPdd->ep[endPoint].pTransfer = NULL;
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
    }
}

//------------------------------------------------------------------------------
//
//  Function:  IssueRxTransfer
//
//  This function receives packet to transaction buffer. It is called from
//  interrupt thread.
//
static VOID IssueRxTransfer(USBFN_PDD *pPdd, DWORD endPoint)
{
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    STransfer *pTransfer;
    BOOL complete = FALSE;
    DWORD space, remain;
    UCHAR *pBuffer;
    DWORD data;
    DWORD stat;
    DWORD epNum;
    DWORD count;
    DWORD maxSize;

    // Get active transfer
    pTransfer = pPdd->ep[endPoint].pTransfer;

    DEBUGMSG(ZONE_TRANSFER, (L"USBD IssueRxTransfer() - "
        L"EP %d pTransfer 0x%08X (%d, %d, %d)\r\n",
                 endPoint,
                 pTransfer,
                 pTransfer != NULL ? pTransfer->cbBuffer : 0,
                 pTransfer != NULL ? pTransfer->cbTransferred : 0,
                 pTransfer != NULL ? pTransfer->dwUsbError : -1));

    // Select EP
    epNum = USBD_EP_NUM & endPoint;
    SelectEp(pPdd, epNum);

    // Get EP status
    stat = INREG32(&pUSBDRegs->STAT_FLG);

    // Depending on EP status
    if ((stat & USBD_STAT_STALL) != 0)
    {
        // We issued stall, remove it...
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_HALT);
    }
    else
    {
        // When transfer is NULL it is handshake ACK
        if (pTransfer != NULL)
        {
            // Get maxPacketSize
            maxSize = pPdd->ep[endPoint].maxPacketSize;

            __try
            {
                pBuffer = (UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
                space = pTransfer->cbBuffer - pTransfer->cbTransferred;

                // Get EP status flag
                stat = INREG32(&pUSBDRegs->STAT_FLG);

                // Get number of bytes in FIFO
                if ((stat & USBD_STAT_FIFO_EMPTY) != 0)
                {
                    count = 0;
                }
                else if ((stat & USBD_STAT_FIFO_FULL) != 0)
                {
                    count = maxSize;
                }
                else
                {
                    count = INREG32(&pUSBDRegs->RXFSTAT) & USBD_RFXSTAT_COUNT;
                }

                // Read data
                remain = count;
                while (remain > 1)
                {
                    data = (WORD)INREG16(&pUSBDRegs->DATA);
                    if (space > 1)
                    {
                        pBuffer[0] = (UCHAR)data;
                        pBuffer[1] = (UCHAR)(data >> 8);
                        pBuffer += 2;
                        space -= 2;
                    }
                    remain -= 2;
                }

                if (remain > 0)
                {
                    data = (WORD)INREG16(&pUSBDRegs->DATA);
                    if (space > 0)
                    {
                        *pBuffer = (UCHAR)data;
                        pBuffer += 1;
                        space   -= 1;
                    }
                    remain -= 1;
                }

                // We transfered some data
                pTransfer->cbTransferred = pTransfer->cbBuffer - space;

                DEBUGMSG(ZONE_TRANSFER, (L"USBD IssueRxTransfer() - "
                    L"Transfered %d bytes on ep 0x%02X", pTransfer->cbTransferred, epNum));

                // Is this end of transfer?
                if ((pTransfer->cbTransferred == pTransfer->cbBuffer) || (count < maxSize))
                {
                    // Yes, set return code
                    pTransfer->dwUsbError = UFN_NO_ERROR;

                    // And complete flag
                    complete = TRUE;
                }
                else
                {
                    // No, enable FIFO for next packet
                    OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_FIFO_EN);
                }

            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;
                complete = TRUE;
            }
        }
    }

    // Deselect EP
    Deselect(pPdd, epNum);

    // If transaction is complete we should tell MDD
    if (complete)
    {
        pPdd->ep[endPoint].pTransfer = NULL;
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
    }
}


//------------------------------------------------------------------------------
//
//  Function:  DevStatEvent
//
//  This function handles device state change interrupts.
//
static VOID DevStatEvent(USBFN_PDD *pPdd)
{
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD state, change;


    DEBUGMSG(ZONE_FUNCTION, (L"USBD DevStatEvent() - START\r\n"));

    // Get Current Device State and compare to Previous Device State
    // Store the differences in change.
    // Note: (a ^ b) == (a XOR b)
    state = INREG32(&pUSBDRegs->DEVSTAT);
    change = state ^ pPdd->devState;

    DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
        L"Previous Device State = 0x%04X\r\n", pPdd->devState));
    DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
        L"Current  Device State = 0x%04X\r\n", state));
    DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
        L"Device State Change   = 0x%04X\r\n", change));

    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd!DevStatEvent: "
        L"Device State = 0x%04x, change = 0x%04X\r\n", state, change));

#ifdef USE_OLD_CABLE_DETECT_METHOD
    // Attach/deattach
    if ((change & USBD_DEVSTAT_ATT) != 0)
    {
        if ((state & USBD_DEVSTAT_ATT) != 0)
        {
            DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
                L"Detected Cable Insertion Event!\r\n"));

            // TODO: Call bus driver (OTG?) to move HW from deep sleep
            // SetSelfPowerState to D0.
            pPdd->m_CurSelfPowerState = D0;
            UpdateDevicePower(pPdd);

            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED,  BS_FULL_SPEED);
        }
        else
        {
            DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
                L"Detected Cable Removal Event!\r\n"));

            // We are not configured anymore
            OUTREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_CLR_CFG);

            // TODO: Call bus driver (OTG?) to move HW to deep sleep
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);

            // Don't process other changes (we are disconnected)

            // SetSelfPowerState to D4
            pPdd->m_CurSelfPowerState = D4; // Do we need set to D3 as wake up source?
            UpdateDevicePower(pPdd);

            goto DevStatClean;
        }
    }
#endif // USE_OLD_CABLE_DETECT_METHOD

    // Reset
    if (((change & USBD_DEVSTAT_USB_RESET) != 0) || ((change & USBD_DEVSTAT_DEF) != 0))
    {
        DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
            L"Detected USB Reset Event!\r\n"));

        pPdd->m_NewPowerState = D0;
        UpdateDevicePower(pPdd);

        if ((state & USBD_DEVSTAT_USB_RESET) == 0)
        {
            // OTG may not detect attach/detach events correctly on some platforms
            // Simulate a attach/detach event to clear any previous state on reset
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED,  BS_FULL_SPEED);

            // Tell MDD about reset...
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_RESET);
        }

        // Enable interrupts
        OUTREG32(&pUSBDRegs->IRQ_EN, USBD_IRQ_MASK);

        // In the middle of a reset don't process other changes
        goto DevStatClean;
    }

    // Suspend/resume
    if ((change & USBD_DEVSTAT_SUS) != 0)
    {
        if ((state & USBD_DEVSTAT_SUS) != 0)
        {
            DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
                L"Detected Device Suspend Event!\r\n"));

            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_SUSPEND);

            // Read device status
            INREG32(&pUSBDRegs->DEVSTAT);

            // Put the device in the D2 state.
            // Note that putting the device in D4 causes an endless loop if
            // no USB cable is currently plugged in. The suspend bit is set
            // when we get a suspend interrupt; if we turn the power off,
            // the suspend bit gets cleared, triggering a resume interrupt.
            // If no USB cable is plugged in, then a suspend interrupt gets
            // generated since no USB device is present. The suspend interrupt
            // then triggers the power off, which clears the suspend bit,
            // which triggers the resume interrupt, and we enter an infinite
            // loop from which there is no escape.
            pPdd->m_NewPowerState = D2;
            UpdateDevicePower(pPdd);

#ifndef USE_OLD_CABLE_DETECT_METHOD

            // This is a work-around for what is fundamentally a hardware design flaw.
            // We can't use the ATT bit in the DEV_STAT register to determine when
            // a cable is removed since the ATT bit is always set since we set the
            // BSESSVLD bit in the InitializeHardware() function. We are supposed to
            // be able to check to see whether a cable is connected to the device
            // and set BSESSVLD as appropriate, but the hardware design for the H4
            // lacks this functionality.
            // As a result, we treat SUSPEND interrupts as if they indicate both a
            // suspend and a cable removal.

            DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
                L"Detected Cable Removal Event!\r\n"));

            // We are not configured anymore
            OUTREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_CLR_CFG);

            // TODO: Call bus driver (OTG?) to move HW to deep sleep
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);

            // Don't process other changes (we are disconnected)
            goto DevStatClean;

#endif // ! USE_OLD_CABLE_DETECT_METHOD
        }
        else
        {
            DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
                L"Detected Device Resume Event!\r\n"));

            pPdd->m_NewPowerState = D0;
            UpdateDevicePower(pPdd);

            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_RESUME);

#ifndef USE_OLD_CABLE_DETECT_METHOD

            // This is a work-around for what is fundamentally a hardware design flaw.
            // We can't use the ATT bit in the DEV_STAT register to determine when
            // a cable is inserted since the ATT bit is always set since we set the
            // BSESSVLD bit in the InitializeHardware() function. We are supposed to
            // be able to check to see whether a cable is connected to the device
            // and set BSESSVLD as appropriate, but the hardware design for the H4
            // lacks this functionality.
            // As a result, we treat RESUME interrupts as if they indicate both a
            // resume and a cable insertion.

            DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
                L"Detected Cable Insertion Event!\r\n"));

            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED,  BS_FULL_SPEED);

#endif // ! USE_OLD_CABLE_DETECT_METHOD
        }
    }

    // Configured
    // if ((change & USBD_DEVSTAT_CFG) != 0)
    // {
    // }

    // Addressed
    if ((change & USBD_DEVSTAT_ADD) != 0)
    {
        DEBUGMSG(ZONE_PDD, (L"USBD DevStatEvent() - "
            L"Detected Device Address Event!\r\n"));

        if ((state & USBD_DEVSTAT_ADD) != 0)
        {
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SET_ADDRESS, 1);
        }
        else
        {
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_CONFIGURED,  0);
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SET_ADDRESS, 0);
        }
    }

DevStatClean:
    // Save device state for next interrupt
    pPdd->devState = state;

    DEBUGMSG(ZONE_FUNCTION, (L"USBD DevStatEvent() - END\r\n"));
}

//------------------------------------------------------------------------------
//
//  Function:  InterruptThread
//
//  This is interrupt thread. It controls responsed to hardware interrupt. To
//  reduce code length it calls interrupt specific functions.
//
static DWORD WINAPI InterruptThread(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD code;
    DWORD source;
    DWORD ep;

    DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - START\r\n"));

    while (!pPdd->exitIntrThread)
    {
        // Wait for interrupt
        code = WaitForSingleObject(pPdd->hIntrEvent, INFINITE);

        DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - "
            L"Interrupt Event Signalled!\r\n"));

        if (code != WAIT_OBJECT_0)
        {
            break;
        }

        // Exit thread when we are told to do so...
        if (pPdd->exitIntrThread)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - "
                L"Exiting Thread\r\n"));
            break;
        }

        if ((pPdd->m_CurrentPowerState == D3) || (pPdd->m_CurrentPowerState == D4))
        {
            pPdd->m_NewPowerState = D2;
            UpdateDevicePower(pPdd);
        }

        // Get interrupt source
        source = INREG32(&pUSBDRegs->IRQ_SRC);

        DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - "
            L"Interrupt Source: 0x%04X\r\n", (source & 0x07BF)));

        // Device state
        if ((source & USBD_INT_DS_CHG) != 0 || pPdd->fakeDsChange)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - "
                L"Processing Device Status Change Interrupt\r\n"));

            // Handle device state change
            DevStatEvent(pPdd);

            // Clear fake DsChange flag
            pPdd->fakeDsChange = FALSE;

            if ((source & USBD_INT_DS_CHG) != 0)
            {
                OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_DS_CHG);
            }
        }

        // EP0 RX interrupt
        if ((source & USBD_INT_EP0_RX) != 0)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - "
                L"Processing Endpoint 0 RX Interrupt\r\n"));

            // Clear source bit
            OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_EP0_RX);

            // Issue next Rx transfer
            IssueRxTransfer(pPdd, 0);
        }

        // EP0 TX interrupt
        if ((source & USBD_INT_EP0_TX) != 0)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - "
                L"Processing Endpoint 0 TX Interrupt\r\n"));

            // Clear source bit
            OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_EP0_TX);

            // Issue next Tx transfer
            IssueTxTransfer(pPdd, 0);
        }

        // EPn RX interrupt
        if ((source & USBD_INT_EP_RX) != 0)
        {
            // Get EP number
            ep = (INREG32(&pUSBDRegs->EP_STAT) >> 8) & USBD_EP_NUM;

            DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - "
                L"Processing Endpoint %d RX Interrupt\r\n", ep));

            // Clear source bit
            OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_EP_RX);

            // Handle event
            IssueRxTransfer(pPdd, ep);
        }

        // EPn TX interrupt
        if ((source & USBD_INT_EP_TX) != 0)
        {
            // Get EP number
            ep = INREG32(&pUSBDRegs->EP_STAT) & USBD_EP_NUM;

            DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - "
                L"Processing Endpoint %d RX Interrupt\r\n", ep));

            // Clear source bit
            OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_EP_TX);

            // Handle event
            IssueTxTransfer(pPdd, ep);
        }

        // Setup Packet
        if ((source & USBD_INT_SETUP) != 0)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - "
                L"Processing Setup Interrupt\r\n"));

            // Reading setup FIFO clears interrupt
            SetupEvent(pPdd);
        }

        // Finish interrupt
        InterruptDone(pPdd->sysIntr);
    }

    DEBUGMSG(ZONE_INTERRUPTS, (L"USBD InterruptThread() - END\r\n"));

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IssueTransfer
//
DWORD WINAPI UfnPdd_IssueTransfer(VOID      * pPddContext,
                                  DWORD       endPoint,
                                  STransfer * pTransfer)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;


    // Save transfer for interrupt thread
    pPdd->ep[endPoint].pTransfer = pTransfer;

    if (pTransfer->pvBuffer == NULL)
    {
        // Sync Length with buffer.
        pTransfer->cbBuffer = 0;
    }

    DEBUGCHK(pTransfer->dwUsbError == UFN_NOT_COMPLETE_ERROR);

    // Depending on direction
    if (TRANSFER_IS_IN(pTransfer))
    {
        pPdd->ep[endPoint].fZeroLengthNeeded = (pTransfer->cbBuffer==0);
        IssueTxTransfer(pPdd, endPoint);
    }
    else
    {
        pPdd->ep[endPoint].fZeroLengthNeeded  = FALSE;

        // Select EP
        epNum = USBD_EP_NUM & endPoint;
        SelectEp(pPdd, epNum);

        // Enable EP FIFO
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_FIFO_EN);

        // Deselect EP
        DeselectEp(pPdd, epNum);
    }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_AbortTransfer
//
DWORD WINAPI UfnPdd_AbortTransfer(VOID *pPddContext, DWORD endPoint, STransfer *pTransfer)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    // Select EP
    epNum = USBD_EP_NUM & endPoint;

    if (TRANSFER_IS_IN(pTransfer))
    {
        epNum |= USBD_EP_NUM_DIRIN;
    }

    SelectEp(pPdd, epNum);

    // Clear EP
    OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_EP);

    // Deselect EP
    DeselectEp(pPdd, epNum);

    // Finish transfer
    pPdd->ep[endPoint].pTransfer = NULL;
    pTransfer->dwUsbError = UFN_CANCELED_ERROR;
    pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_StallEndpoint
//
//  This function is called by MDD to set end point to stall mode (halted).
//
DWORD WINAPI UfnPdd_StallEndpoint(VOID *pPddContext, DWORD endPoint)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd_StallEndpoint %d\r\n", endPoint));

    if (endPoint == 0)
    {
        // Stall next EP0 transaction
        OUTREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_STALL_CMD);

    }
    else
    {
        // Select EP
        epNum = USBD_EP_NUM & endPoint;

        if (!pPdd->ep[endPoint].dirRx)
        {
            epNum |= USBD_EP_NUM_DIRIN;
        }

        SelectEp(pPdd, epNum);

        // Halt EP
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_SET_HALT);

        // Deselect EP
        DeselectEp(pPdd, epNum);
    }

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_ClearEndpointStall
//
//  This function is called by MDD to clear end point stall mode (halted).
//
DWORD WINAPI UfnPdd_ClearEndpointStall(VOID *pPddContext, DWORD endPoint)
{
    DWORD rc = ERROR_INVALID_FUNCTION;
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd_ClearEndpoint %d\r\n", endPoint));

    // Endpoint can't be zero
    if (endPoint != 0)
    {
        // Select EP
        epNum = USBD_EP_NUM & endPoint;

        if (!pPdd->ep[endPoint].dirRx)
        {
            epNum |= USBD_EP_NUM_DIRIN;
        }

        SelectEp(pPdd, epNum);

        // Clear suspend, Clear data toggle
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_HALT | USBD_CTRL_CLR_DATA_TOGGLE);

        // Deselect EP
        DeselectEp(pPdd, epNum);

        // Done
        rc = ERROR_SUCCESS;
    }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IsEndpointHalted
//
DWORD WINAPI UfnPdd_IsEndpointHalted(VOID *pPddContext,
                                     DWORD endPoint,
                                     BOOL *pHalted)
{
    DWORD rc = ERROR_INVALID_FUNCTION;
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd_IsEndpointHalted %d\r\n", endPoint));

    // Endpoint can't be zero
    if (endPoint != 0)
    {
        // Select EP
        epNum = USBD_EP_NUM & endPoint;

        if (!pPdd->ep[endPoint].dirRx)
        {
            epNum |= USBD_EP_NUM_DIRIN;
        }

        SelectEp(pPdd, epNum);

        // Is EP halted?
        *pHalted = ((INREG32(&pUSBDRegs->STAT_FLG) & USBD_STAT_HALTED) != 0);

        // Deselect EP
        DeselectEp(pPdd, epNum);

        // Done
        rc = ERROR_SUCCESS;
    }

    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_SendControlStatusHandshake
//
//  Send the control status handshake.
//
DWORD WINAPI UfnPdd_SendControlStatusHandshake(VOID *pPddContext,
                                               DWORD endPoint)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum, stat;

    DEBUGMSG(ZONE_USB_EVENTS, (L"UfnPdd_SendControlStatusHandshake() - START\r\n"));

    // Select EP
    epNum = USBD_EP_NUM & endPoint;

    if (pPdd->setupDirRx)
    {
        epNum |= USBD_EP_NUM_DIRIN;
    }

    SelectEp(pPdd, epNum);

    // Get actual status
    stat = INREG32(&pUSBDRegs->STAT_FLG);

    DEBUGMSG(ZONE_USB_EVENTS, (L"UfnPdd_SendControlStatusHandshake() - "
        L"EndPoint %d (%s): STAT_FLG = 0x%04X\r\n", endPoint, pPdd->setupDirRx ? L"IN" : L"OUT", stat));

    // Only send the handshake when the EndPoint is not stalled.
    if ((stat & USBD_STAT_STALL) == 0)
    {
        // Clear & enable FIFO
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_EP);
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_FIFO_EN);
    }
    else
    {
        DEBUGMSG(ZONE_USB_EVENTS, (L"UfnPdd_SendControlStatusHandshake() - "
            L"Not sending handshake because Endpoint %d is stalled\r\n", epNum));
    }

    // Deselect EP
    DeselectEp(pPdd, epNum);

    DEBUGMSG(ZONE_USB_EVENTS, (L"UfnPdd_SendControlStatusHandshake() - END\r\n"));

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_InitiateRemoteWakeup
//
DWORD WINAPI UfnPdd_InitiateRemoteWakeup(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;

    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd_InitiateRemoteWakeup\r\n"));

    SETREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_RMT_WKP);

    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------

#define PSZ_REG_CLIENT_DRIVER_PATH              _T("\\Drivers\\USB\\FunctionDrivers")
#define PSZ_REG_DEFAULT_DEFAULT_CLIENT_DRIVER   _T("DefaultClientDriver")
#define PSZ_REG_FRIENDLY_NAME                   _T("FriendlyName")
#define PSZ_REG_DEFAULT_CLIENT_KEY              _T("DefaultClientKey")
#define MAX_LOADSTRING                          100

static BOOL GetUfnDescription(HKEY   hkClient,
                              LPTSTR pszDescription,
                              DWORD  cchDescription)
{
    DWORD dwType;
    DWORD cbValue = sizeof(TCHAR) * cchDescription;
    DWORD dwError = RegQueryValueEx(hkClient,
                                    PSZ_REG_FRIENDLY_NAME,
                                    NULL,
                                    &dwType,
                                    (PBYTE)pszDescription,
                                    &cbValue);

    pszDescription[cchDescription - 1] = 0; // Null-terminate

    if ((dwError != ERROR_SUCCESS) || (dwType != REG_SZ))
    {
        // No description. Still return success, though.
        pszDescription[0] = 0;
    }

    return TRUE;
}


static DWORD OpenFunctionKey(HKEY *phkFunctions)
{
    // Determine which client driver to load
    return RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        PSZ_REG_CLIENT_DRIVER_PATH,
                        0,
                        0,
                        phkFunctions);
}


static DWORD GetDefaultClientName(LPTSTR pszClientName)
{
    HKEY  hkFunctions = NULL;
    DWORD cbData      = MAX_LOADSTRING * sizeof(TCHAR);
    DWORD dwType;
    DWORD dwRet       = OpenFunctionKey(&hkFunctions);


    if (dwRet == ERROR_SUCCESS)
    {
        dwRet = RegQueryValueEx(hkFunctions,
                                PSZ_REG_DEFAULT_DEFAULT_CLIENT_DRIVER,
                                NULL,
                                &dwType,
                                (PBYTE)pszClientName,
                                &cbData);

        if ((dwRet != ERROR_SUCCESS) || (dwType != REG_SZ))
        {
            // No client name. Still return success, though.
            pszClientName[0] = 0;
        }
    }

    if (hkFunctions)
    {
        RegCloseKey(hkFunctions);
    }

    return dwRet;
}


static DWORD ChangeDefaultClient(LPTSTR pszClientName)
{
    DWORD dwRet;
    HKEY  hkFunctions = NULL;


    dwRet = OpenFunctionKey(&hkFunctions);
    if (dwRet == ERROR_SUCCESS)
    {
        dwRet = RegSetValueEx(hkFunctions,
                              PSZ_REG_DEFAULT_DEFAULT_CLIENT_DRIVER,
                              0,
                              REG_SZ,
                              (PBYTE)pszClientName,
                              (_tcslen(pszClientName) + 1) * sizeof(TCHAR));
    }

    if (hkFunctions)
    {
        RegCloseKey(hkFunctions);
    }

    return dwRet;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IOControl
//
DWORD WINAPI UfnPdd_IOControl(VOID        *pPddContext,
                              IOCTL_SOURCE source,
                              DWORD        code,
                              UCHAR       *pInBuffer,
                              DWORD        inSize,
                              UCHAR       *pOutBuffer,
                              DWORD        outSize,
                              DWORD       *pOutSize)
{
    DWORD                rc   = ERROR_INVALID_PARAMETER;
    USBFN_PDD           *pPdd = pPddContext;
    UFN_PDD_INFO         info;
    CE_BUS_POWER_STATE  *pBusPowerState;
    CEDEVICE_POWER_STATE devicePowerState;


    switch (code)
    {
        case IOCTL_UFN_GET_PDD_INFO:
            if (source != BUS_IOCTL)
            {
                break;
            }
            if (pOutBuffer == NULL || outSize < sizeof(UFN_PDD_INFO))
            {
                break;
            }
            info.InterfaceType = Internal;
            info.BusNumber = 0;
            info.dwAlignment = sizeof(DWORD);
            if (!CeSafeCopyMemory(pOutBuffer, &info, sizeof(UFN_PDD_INFO)))
            {
                break;
            }
            rc = ERROR_SUCCESS;
            break;

        case IOCTL_BUS_GET_POWER_STATE:
            if (source != MDD_IOCTL)
            {
                break;
            }
            if (pInBuffer == NULL || inSize < sizeof(CE_BUS_POWER_STATE))
            {
                break;
            }
            pBusPowerState = (CE_BUS_POWER_STATE*)pInBuffer;
            if (!CeSafeCopyMemory(pBusPowerState->lpceDevicePowerState, &pPdd->m_CurrentPowerState, sizeof(CEDEVICE_POWER_STATE)))
            {
                break;
            }

            rc = ERROR_SUCCESS;
            break;

        case IOCTL_BUS_SET_POWER_STATE:
            if (pInBuffer == NULL || inSize < sizeof(CE_BUS_POWER_STATE))
            {
                DEBUGMSG(ZONE_WARNING, (L"UfnPdd_IOControl() "
                    L"WARNING: Bad Parameter\r\n"));
                break;
            }
            pBusPowerState = (CE_BUS_POWER_STATE*)pInBuffer;
            if (!CeSafeCopyMemory(&devicePowerState, pBusPowerState->lpceDevicePowerState, sizeof(CEDEVICE_POWER_STATE)))
            {
                break;
            }
            DEBUGMSG(ZONE_POWER, (L"UfnPdd_IOControl() - "
                L"Set Power State to D%d\r\n", (int)devicePowerState));
            pPdd->m_NewPowerState = devicePowerState;
            if (UpdateDevicePower(pPdd))
            {
                rc = ERROR_SUCCESS;
            }
            break;

        case IOCTL_UFN_CHANGE_DEFAULT_CLIENT:
            rc = ChangeDefaultClient((LPTSTR)pInBuffer);
            break;
    }

    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Deinit
//
DWORD WINAPI UfnPdd_Deinit(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;


    // Stop interrupt thread
    if (pPdd->hIntrThread != NULL)
    {
        pPdd->exitIntrThread = TRUE;
        SetEvent(pPdd->hIntrEvent);
        WaitForSingleObject(pPdd->hIntrThread, INFINITE);
        CloseHandle(pPdd->hIntrThread);
    }

    // Close interrupt handler
    if (pPdd->hIntrEvent != NULL)
    {
        CloseHandle(pPdd->hIntrEvent);
        pPdd->hIntrEvent = NULL;
    }

    // If parent bus is open, set hardware to D4 and close it
    if (pPdd->hParentBus != NULL)
    {
        SetDevicePowerState(pPdd->hParentBus, D4, NULL);
        CloseBusAccessHandle(pPdd->hParentBus);
        pPdd->hParentBus = NULL;
    }

    // Unmap USBD controller registers
    if (pPdd->pUSBDRegs != NULL)
    {
        MmUnmapIoSpace((VOID*)pPdd->pUSBDRegs, pPdd->memLen);
        pPdd->pUSBDRegs = NULL;
    }

    // Release interrupt
    if (pPdd->sysIntr != 0)
    {
        InterruptDisable(pPdd->sysIntr);
        KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR,
                        &pPdd->sysIntr,
                        sizeof(pPdd->sysIntr),
                        NULL,
                        0,
                        NULL);

        pPdd->sysIntr = 0;
    }

    // Delete critical section
    DeleteCriticalSection(&pPdd->epCS);

    // Free PDD context
    LocalFree(pPdd);

    AdvertiseInterface((const GUID *)DMCLASS_PROTECTEDBUSNAMESPACE, L"UsbFn",FALSE);

    // Done
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_DeregisterDevice
//
//  This function is called by MDD to move device to pre-registred state.
//  On OMAP7xx we simply disable all end points.
//
DWORD WINAPI UfnPdd_DeregisterDevice(VOID *pPddContext)
{
    USBFN_PDD          *pPdd      = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD               ep;


    // Disable all RX, TX EPs
    OUTREG32(&pUSBDRegs->EP0, 0);

    for (ep = 0; ep < USBD_NONZERO_EP_COUNT; ep++)
    {
        OUTREG32(&pUSBDRegs->EP_RX[ep], 0);
        OUTREG32(&pUSBDRegs->EP_TX[ep], 0);
    }

    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Stop
//
//  This function is called before UfnPdd_DeregisterDevice. It should de-attach
//  device to USB bus (but we don't want disable interrupts because...)
//
DWORD WINAPI UfnPdd_Stop(VOID *pPddContext)
{
    USBFN_PDD           *pPdd      = pPddContext;
    OMAP2420_USBD_REGS  *pUSBDRegs = pPdd->pUSBDRegs;
    CEDEVICE_POWER_STATE prevPowerState ;


    DEBUGMSG(ZONE_PDD, (L"UfnPdd_Stop\r\n"));

    prevPowerState = pPdd->m_CurrentPowerState;

    if ((prevPowerState == D3) || (prevPowerState == D4))
    {
        pPdd->m_NewPowerState = D2;
        UpdateDevicePower(pPdd);
    }

    // Deattach device
    CLRREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_PULLUP_EN);

    pPdd->m_NewPowerState = prevPowerState;
    UpdateDevicePower(pPdd);

    // Done
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_DeinitEndpoint
//
//  This function is called when pipe to endpoint is closed. For OMAP7XX we
//  will stop points in UfnPdd_DeregisterDevice.
//
DWORD WINAPI UfnPdd_DeinitEndpoint(VOID *pPddContext, DWORD endPoint)
{
    USBFN_PDD          *pPdd      = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD               epNum;


    DEBUGMSG(ZONE_PDD, (L"UfnPdd_DeinitEndpoint: %d\r\n", endPoint));

    // Select EP
    epNum = USBD_EP_NUM & endPoint;
    SelectEp(pPdd, epNum);

    // Clear EP
    OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_EP);

    // Deselect EP
    DeselectEp(pPdd, epNum);

    // Done
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_InitEndpoint
//
//  This function is called when pipe to endpoint is created. For OMAP7XX
//  all initialization must be done in UfnPdd_RegisterDevice.
//
DWORD WINAPI UfnPdd_InitEndpoint(VOID *pContext,
                                 DWORD endPoint,
                                 UFN_BUS_SPEED speed,
                                 USB_ENDPOINT_DESCRIPTOR *pEPDesc,
                                 VOID *pReserved,
                                 UCHAR configValue,
                                 UCHAR interfaceNumber,
                                 UCHAR alternateSetting)
{
    DEBUGMSG(ZONE_PDD, (L"UfnPdd_InitEndpoint: %d\r\n", endPoint));
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_SetAddress
//
//  This function is called by MDD when configuration process assigned address
//  to device. For OMAP7xx this is managed by hardware.
//
DWORD WINAPI UfnPdd_SetAddress(VOID *pPddContext, UCHAR address)
{
    DEBUGMSG(ZONE_PDD, (L"UfnPdd_SetAddress(0x%08X, 0x%02X)\r\n", pPddContext, address));
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Start
//
//  This function is called after UfnPdd_RegisterDevice. It should attach
//  device to USB bus.
//
DWORD WINAPI UfnPdd_Start(VOID *pPddContext)
{
    DWORD rv;
    
    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_Start() - START\r\n"));

    if (pPddContext == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Start() - "
            L"Invalid Parameter: pPddContext is NULL\r\n"));
        rv = ERROR_INVALID_PARAMETER;
    }
    else
    {
        USBFN_PDD          * pPdd           = pPddContext;
        OMAP2420_USBD_REGS * pUSBDRegs      = pPdd->pUSBDRegs;


        pPdd->m_NewPowerState = D2;
        UpdateDevicePower(pPdd);

        // Enable interrupts
        OUTREG32(&pUSBDRegs->IRQ_EN, USBD_IRQ_MASK);

        // Attach device to bus (it has no effect when OTG controller is used)
        SETREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_PULLUP_EN);

        // Set fake device change flag which on first interrupt force
        // device state change handler even if it isn't indicated by hardware
        pPdd->fakeDsChange = TRUE;

        rv = ERROR_SUCCESS;
    }
    

    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_Start() - END\r\n"));

    // Done
    return rv;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_RegisterDevice
//
//  This function is called by MDD after device configuration was sucessfully
//  verified by UfnPdd_IsEndpointSupportable and
//  UfnPdd_IsConfigurationSupportable. It should initialize hardware for given
//  configuration. Depending on hardware endpoints can be initialized later in
//  UfnPdd_InitEndpoint. For OMAP7xx it isn't a case, so we should do all
//  initialization there.
//
DWORD WINAPI UfnPdd_RegisterDevice(VOID                               *pPddContext,
                                   const USB_DEVICE_DESCRIPTOR        *pHighSpeedDeviceDesc,
                                   const UFN_CONFIGURATION            *pHighSpeedConfig,
                                   const USB_CONFIGURATION_DESCRIPTOR *pHighSpeedConfigDesc,
                                   const USB_DEVICE_DESCRIPTOR        *pFullSpeedDeviceDesc,
                                   const UFN_CONFIGURATION            *pFullSpeedConfig,
                                   const USB_CONFIGURATION_DESCRIPTOR *pFullSpeedConfigDesc,
                                   const UFN_STRING_SET               *pStringSets,
                                   DWORD stringSets)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT  *pEP;
    DWORD offset, ep;
    DWORD ifc, epx;
    DWORD cfg;


    DEBUGMSG(ZONE_FUNCTION, (L"UfnPdd_RegisterDevice() - START\r\n"));

    // Remember self powered flag
    pPdd->selfPowered = ((pFullSpeedConfig->Descriptor.bmAttributes & 0x20) != 0);

    // Unlock configuration
    CLRREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_CFG_LOCK);

    DEBUGMSG(ZONE_PDD, (L"UfnPdd_RegisterDevice() - "
        L"Configuring EndPoint 00 (RX)\r\n"));

    // Configure EP0
    offset = 8;
    cfg  = (Log2(pFullSpeedDeviceDesc->bMaxPacketSize0 >> 3) << 12);
    cfg |= offset >> 3;

    OUTREG32(&pUSBDRegs->EP0, cfg);

    pPdd->ep[0].maxPacketSize = pFullSpeedDeviceDesc->bMaxPacketSize0;
    offset += pFullSpeedDeviceDesc->bMaxPacketSize0;

    // Configure Rx EPs
    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface...
        pIFC = &pFullSpeedConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
        {
            pEP = &pIFC->pEndpoints[epx];

            // If it is Tx EP skip it
            if ((pEP->Descriptor.bEndpointAddress & 0x80) != 0)
            {
                continue;
            }

            DEBUGMSG(ZONE_PDD, (L"UfnPdd_RegisterDevice() - "
                L"Configuring Interface %02u EndPoint %02u (RX)\r\n", ifc, epx));

            // Get EP address
            ep = pEP->Descriptor.bEndpointAddress & 0x0F;

            // Save max packet size & direction
            pPdd->ep[ep].maxPacketSize = pEP->Descriptor.wMaxPacketSize;
            pPdd->ep[ep].dirRx = TRUE;

            // Create EP config
            cfg  = USBD_EP_VALID;
            cfg |= (Log2(pEP->Descriptor.wMaxPacketSize >> 3) << 12);

            if ((pEP->Descriptor.bmAttributes & 0x03) == 0x01)
            {
                cfg |= USBD_EP_ISO;
            }

            cfg |= offset >> 3;

            if (ep > 0)
                OUTREG32(&pUSBDRegs->EP_RX[ep - 1], cfg);
            else
                DEBUGMSG(ZONE_PDD, (L"USBFN:: UfnPdd_RegisterDevice EP_RX = %d\r\n", ep - 1));

            // Update offset
            offset += pEP->Descriptor.wMaxPacketSize;
        }
    }

    // Configure Tx EPs
    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pFullSpeedConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
        {
            pEP = &pIFC->pEndpoints[epx];

            // If it is Rx EP skip it
            if ((pEP->Descriptor.bEndpointAddress & 0x80) == 0)
            {
                continue;
            }

            DEBUGMSG(ZONE_PDD, (L"UfnPdd_RegisterDevice() - "
                L"Configuring Interface %02u EndPoint %02u (TX)\r\n", ifc, epx));

            // Get EP address
            ep = pEP->Descriptor.bEndpointAddress & 0x0F;

            // Save max packet size & direction
            pPdd->ep[ep].maxPacketSize = pEP->Descriptor.wMaxPacketSize;
            pPdd->ep[ep].dirRx = FALSE;

            // Create EP config
            cfg  = USBD_EP_VALID;

            cfg |= (Log2(pEP->Descriptor.wMaxPacketSize >> 3) << 12);

            if ((pEP->Descriptor.bmAttributes & 0x03) == 0x01)
            {
                cfg |= USBD_EP_ISO;
            }

            cfg |= offset >> 3;

            if (ep > 0)
                OUTREG32(&pUSBDRegs->EP_TX[ep - 1], cfg);
            else
                DEBUGMSG(ZONE_PDD, (L"USBFN:: UfnPdd_RegisterDevice "
                    L"EP_TX = %d\r\n", ep - 1));


            // Update offset
            offset += pEP->Descriptor.wMaxPacketSize;
        }
    }

    // Lock configuration
    SETREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_CFG_LOCK);

    DEBUGMSG(ZONE_PDD, (L"UfnPdd_RegisterDevice() - END\r\n"));

    // Done
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IsEndpointSupportable
//
//  This function is called by MDD to verify if EP can be supported on
//  hardware. It is called after UfnPdd_IsConfigurationSupportable. We must
//  verify configuration in this function, so we already know that EPs
//  are valid. Only information we can update there is maximal packet
//  size for EP0.
//
DWORD WINAPI UfnPdd_IsEndpointSupportable(VOID *pPddContext,
                                          DWORD endPoint,
                                          UFN_BUS_SPEED speed,
                                          USB_ENDPOINT_DESCRIPTOR *pEPDesc,
                                          UCHAR configurationValue,
                                          UCHAR interfaceNumber,
                                          UCHAR alternateSetting)
{
    USBFN_PDD *pPdd = pPddContext;


    // Update maximal packet size for EP0
    if (endPoint == 0)
    {
        DEBUGCHK(pEPDesc->wMaxPacketSize <= 64);
        DEBUGCHK(pEPDesc->bmAttributes == USB_ENDPOINT_TYPE_CONTROL);
        pEPDesc->wMaxPacketSize = 64;
    }

    // Done
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IsConfigurationSupportable
//
//  This function is called before UfnPdd_RegisterDevice. It should verify
//  that USB device configuration can be supported on hardware. Function can
//  modify EP size and/or EP address.
//
//  For OMAP7xx we should check if total descriptor size is smaller
/// than 2040 bytes and round EP sizes. Unfortunately we don't get information
//  about EP0 max packet size. So we will assume maximal 64 byte size.
//
DWORD WINAPI UfnPdd_IsConfigurationSupportable(VOID              * pPddContext,
                                               UFN_BUS_SPEED       speed,
                                               UFN_CONFIGURATION * pConfig)
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    USBFN_PDD *pPdd = pPddContext;
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT *pEP;
    DWORD ifc, epx, count;
    DWORD offset, size;


    // TODO: Update self power bit & maxPower

    // We must start with offset 8 + 64 (config plus EP0 size)
    offset = 8 + 64;

    // Clear number of end points
    count = 0;

    // For each interface in configuration
    for (ifc = 0; ifc < pConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pConfig->pInterfaces[ifc];

        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
        {
            pEP = &pIFC->pEndpoints[epx];

            // We support maximal sizes 8, 16, 32 and 64 bytes for non-ISO
            size = pEP->Descriptor.wMaxPacketSize;

            // First round size to supported sizes
            size = 1 << Log2(size);

            // Is it ISO end point?
            if ((pEP->Descriptor.bmAttributes & 0x03) != 0x01)
            {
                // Non-ISO, max size is 64 bytes
                if (size > 64)
                {
                    size = 64;
                }
            }
            else
            {
                // ISO edpoint, maximal size is 512 bytes
                if (size > 512)
                {
                    size = 512;
                }
            }

            // Update EP size
            pEP->Descriptor.wMaxPacketSize = (USHORT)size;

            // Calculate total buffer size
            offset += size;
        }

        // Add number of end points to total count
        count += pIFC->Descriptor.bNumEndpoints;
    }

    // Can we support this configuration?
    if (count < USBD_EP_COUNT && offset <= 2048)
    {
        rc = ERROR_SUCCESS;
    }

    // Done
    return rc;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Init
//
//  This function is called by MDD on driver load. It should reset driver,
//  fill PDD interface structure. It can also get SYSINTR, initialize interrupt
//  thread and start interrupt thread. It must not attach device to USB bus.
//
DWORD WINAPI UfnPdd_Init(LPCTSTR                  szActiveKey,
                         VOID                   * pMddContext,
                         UFN_MDD_INTERFACE_INFO * pMddIfc,
                         UFN_PDD_INTERFACE_INFO * pPddIfc)
{
    DWORD                rc = ERROR_INVALID_PARAMETER;
    DWORD                ep;
    USBFN_PDD          * pPdd;
    OMAP2420_USBD_REGS * pUSBDRegs;
    PHYSICAL_ADDRESS     pa;
	DWORD                irqs[5];


    DEBUGMSG(ZONE_INIT, (L"UfnPdd_Init() - START\r\n"));

    printUsbdDriverSettings();

    DEBUGMSG(ZONE_INIT, (L"UfnPdd_Init() - "
        L"Initializing Hardware...\r\n"));

    if (InitializeHardware() == FALSE)
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Init() - "
            L"ERROR: Hardware Initialization FAILED\r\n"));
        goto InitClean;
    }
    // Allocate PDD object
    pPdd = LocalAlloc(LPTR, sizeof(USBFN_PDD));
    if (pPdd == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Init() - "
            L"ERROR: Could not allocate memory for pPdd\r\n"));
        goto InitClean;
    }

    // Initialize critical section
    InitializeCriticalSection(&pPdd->epCS);
    pPdd->devState = 0;

    // Read device parameters
    if (GetDeviceRegistryParams(szActiveKey, pPdd, dimof(g_deviceRegParams), g_deviceRegParams) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Init() - "
            L"ERROR: Could not get Device Registry Parameters\r\n"));
        goto InitClean;
    }

    ASSERT(pPdd->memBase == OMAP2420_USBD_REGS_PA);
    ASSERT(pPdd->memLen >= sizeof(OMAP2420_USBD_REGS));

    pPdd->irq[0] = IRQ_USB_GEN;
    pPdd->irq[1] = IRQ_USB_NISO;
    pPdd->irq[2] = IRQ_USB_ISO;

    // Set PM to Default
    pPdd->m_NewPowerState     = D0;
    pPdd->m_CurrentPowerState = D4;

    pPdd->hParentBus = CreateBusAccessHandle(szActiveKey);
    if (pPdd->hParentBus == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Init() - "
            L"ERROR: pPdd->hParentBus is NULL\r\n"));
        goto InitClean;
    }

    // Set hardware to standby mode
    pPdd->m_NewPowerState = D2;
    UpdateDevicePower(pPdd);

    // Map the USB OHCI registers
    pa.QuadPart = pPdd->memBase;
    pUSBDRegs = MmMapIoSpace(pa, pPdd->memLen, FALSE);

    if (pUSBDRegs == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Init() - "
            L"ERROR: Could not map memory for USBD Registers\r\n"));
        goto InitClean;
    }
    pPdd->pUSBDRegs = pUSBDRegs;

    DEBUGMSG((ZONE_INIT && ZONE_INTERRUPTS), (L"UfnPdd_Init() - Setting up interrupts...\r\n"));

    // Clear USB Interrupt enable registers
    OUTREG32(&pUSBDRegs->IRQ_EN, 0);
    OUTREG32(&pUSBDRegs->DMA_IRQ_EN, 0);

    // Reset all interrupts
    OUTREG32(&pUSBDRegs->IRQ_SRC, 0xFFFFFFFF);

    // Disable all RX and TX EndPoints (RX 0 - 15, TX 1 - 15)
    OUTREG32(&pUSBDRegs->EP0, 0);
    for (ep = 0; ep < USBD_NONZERO_EP_COUNT; ep++)
    {
        OUTREG32(&pUSBDRegs->EP_RX[ep], 0);
        OUTREG32(&pUSBDRegs->EP_TX[ep], 0);
    }

    // Request SYSINTR for interrupts
    irqs[0] = -1;                   // We are using new call format
    irqs[1] = OAL_INTR_FORCE_STATIC;      // Mapping flags
    irqs[2] = pPdd->irq[0];         // Device state, EP0 & DMA
    irqs[3] = pPdd->irq[1];         // Non-ISO EP
    irqs[4] = pPdd->irq[2];         // ISO EP
    pPdd->m_NewPowerState = D4;     // Assume Detached First.
    UpdateDevicePower(pPdd);

    DEBUGMSG((ZONE_INIT && ZONE_INTERRUPTS), (L"UfnPdd_Init() - "
        L"Mapping IRQs %d, %d, and %d to System Interrupt...\r\n", irqs[2], irqs[3], irqs[4]));
    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, irqs, sizeof(irqs), &pPdd->sysIntr, sizeof(pPdd->sysIntr), NULL))
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Init() - "
            L"ERROR: System IRQ Request FAILED\r\n"));
        goto InitClean;
    }
    else
    {
        DEBUGMSG((ZONE_INIT && ZONE_INTERRUPTS), (L"UfnPdd_Init() - "
            L"System Interrupt is 0x%08X\r\n", pPdd->sysIntr));
    }

    // Create interrupt event
    pPdd->hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (pPdd->hIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Init() - "
            L"ERROR: Could not create interrupt notification event\r\n"));
        goto InitClean;
    }

    DEBUGMSG((ZONE_INIT && ZONE_INTERRUPTS), (L"UfnPdd_Init() - "
        L"Associating Interrupt Event with System Interrupt 0x%08X...\r\n", pPdd->sysIntr));

    // Initialize interrupt
    if (!InterruptInitialize(pPdd->sysIntr, pPdd->hIntrEvent, NULL, 0))
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Init() - "
            L"ERROR: Could not initialize interrupt notification event\r\n"));
        goto InitClean;
    }

    // Run interrupt thread
    pPdd->exitIntrThread = FALSE;
    pPdd->hIntrThread    = CreateThread(NULL, 0, InterruptThread, pPdd, 0, NULL);

    if (pPdd->hIntrThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"UfnPdd_Init() - "
            L"ERROR: Could not create Interrupt Thread\r\n"));
        goto InitClean;
    }
    CeSetThreadPriority(pPdd->hIntrThread, pPdd->priority256);

    // Set PDD interface
    pPddIfc->dwVersion         = UFN_PDD_INTERFACE_VERSION;
    pPddIfc->dwCapabilities    = UFN_PDD_CAPS_SUPPORTS_FULL_SPEED;
    pPddIfc->dwEndpointCount   = USBD_EP_COUNT;
    pPddIfc->pvPddContext      = pPdd;
    pPddIfc->pfnDeinit         = UfnPdd_Deinit;
    pPddIfc->pfnIsConfigurationSupportable = UfnPdd_IsConfigurationSupportable;
    pPddIfc->pfnIsEndpointSupportable      = UfnPdd_IsEndpointSupportable;
    pPddIfc->pfnInitEndpoint   = UfnPdd_InitEndpoint;
    pPddIfc->pfnRegisterDevice = UfnPdd_RegisterDevice;
    pPddIfc->pfnDeregisterDevice           = UfnPdd_DeregisterDevice;
    pPddIfc->pfnStart          = UfnPdd_Start;
    pPddIfc->pfnStop           = UfnPdd_Stop;
    pPddIfc->pfnIssueTransfer  = UfnPdd_IssueTransfer;
    pPddIfc->pfnAbortTransfer  = UfnPdd_AbortTransfer;
    pPddIfc->pfnDeinitEndpoint = UfnPdd_DeinitEndpoint;
    pPddIfc->pfnStallEndpoint  = UfnPdd_StallEndpoint;
    pPddIfc->pfnClearEndpointStall         = UfnPdd_ClearEndpointStall;
    pPddIfc->pfnSendControlStatusHandshake = UfnPdd_SendControlStatusHandshake;
    pPddIfc->pfnSetAddress     = UfnPdd_SetAddress;
    pPddIfc->pfnIsEndpointHalted           = UfnPdd_IsEndpointHalted;
    pPddIfc->pfnInitiateRemoteWakeup       = UfnPdd_InitiateRemoteWakeup;
    pPddIfc->pfnPowerDown      = UfnPdd_PowerDown;
    pPddIfc->pfnPowerUp        = UfnPdd_PowerUp;
    pPddIfc->pfnIOControl      = UfnPdd_IOControl;

    // Save MDD context & notify function
    pPdd->pMddContext = pMddContext;
    pPdd->pfnNotify   = pMddIfc->pfnNotify;

    // Done
    rc = ERROR_SUCCESS;

InitClean:
    DEBUGMSG(ZONE_INIT, (_T("UfnPdd_Init() - END\r\n")));

    if ( (rc != ERROR_SUCCESS) && (pPdd != NULL) )
    {
            LocalFree(pPdd);
    }

    return rc;
}

//------------------------------------------------------------------------------

extern BOOL UfnPdd_DllEntry(HANDLE hDllHandle, DWORD reason, VOID *pReserved)
{
//    DEBUGMSG(ZONE_PDD, (L"UfnPdd_DllEntry() - CALLED\r\n"));
    return TRUE;
}


#define SHOW_OTG_REGISTERS
#define SHOW_CONFIG_REGISTERS


void PDD_Dump(OMAP2420_USBD_REGS * pPddRegs)
{
    DEBUGMSG(ZONE_PDD, (L"============================================================\r\n"));
    DEBUGMSG(ZONE_PDD, (L"\r\n"));

    if (pPddRegs == NULL)
    {
        DEBUGMSG(ZONE_PDD, (L"USBD PDD_Dump() - ERROR: Pointer to USBD Registers is NULL\r\n"));
    }
    else
    {
        // Print out the values of all USB Device Registers
        DEBUGMSG(ZONE_PDD, (L"Dump USBD Registers:\r\n"));
        DEBUGMSG(ZONE_PDD, (L"\tREV        = 0x%04X\r\n", INREG32(&pPddRegs->REV)));        // 0000 - Revision
        DEBUGMSG(ZONE_PDD, (L"\tEP_NUM     = 0x%04X\r\n", INREG32(&pPddRegs->EP_NUM)));     // 0004 - Endpoint selection
//      DEBUGMSG(ZONE_PDD, (L"\tDATA       = 0x%04X\r\n", INREG32(&pPddRegs->DATA)));       // 0008 - Data
        DEBUGMSG(ZONE_PDD, (L"\tCTRL       = 0x%04X\r\n", INREG32(&pPddRegs->CTRL)));       // 000C - Control
        DEBUGMSG(ZONE_PDD, (L"\tSTAT_FLG   = 0x%04X\r\n", INREG32(&pPddRegs->STAT_FLG)));   // 0010 - Status
        DEBUGMSG(ZONE_PDD, (L"\tRXFSTAT    = 0x%04X\r\n", INREG32(&pPddRegs->RXFSTAT)));    // 0014 - Receive FIFO status
        DEBUGMSG(ZONE_PDD, (L"\tSYSCON1    = 0x%04X\r\n", INREG32(&pPddRegs->SYSCON1)));    // 0018 - System configuration 1
        DEBUGMSG(ZONE_PDD, (L"\tSYSCON2    = 0x%04X\r\n", INREG32(&pPddRegs->SYSCON2)));    // 001C - System configuration 2
        DEBUGMSG(ZONE_PDD, (L"\tDEVSTAT    = 0x%04X\r\n", INREG32(&pPddRegs->DEVSTAT)));    // 0020 - Device status
        DEBUGMSG(ZONE_PDD, (L"\tSOF        = 0x%04X\r\n", INREG32(&pPddRegs->SOF)));        // 0024 - Start of frame
        DEBUGMSG(ZONE_PDD, (L"\tIRQ_EN     = 0x%04X\r\n", INREG32(&pPddRegs->IRQ_EN)));     // 0028 - Interrupt enable
        DEBUGMSG(ZONE_PDD, (L"\tDMA_IRQ_EN = 0x%04X\r\n", INREG32(&pPddRegs->DMA_IRQ_EN))); // 002C - DMA interrupt enable
        DEBUGMSG(ZONE_PDD, (L"\tIRQ_SRC    = 0x%04X\r\n", INREG32(&pPddRegs->IRQ_SRC)));    // 0030 - Interrupt source
        DEBUGMSG(ZONE_PDD, (L"\tEP_STAT    = 0x%04X\r\n", INREG32(&pPddRegs->EP_STAT)));    // 0034 - Non-ISO endpoint interrupt enable
        DEBUGMSG(ZONE_PDD, (L"\tDMA_STAT   = 0x%04X\r\n", INREG32(&pPddRegs->DMA_STAT)));   // 0038 - Non-ISO DMA interrupt enable
        DEBUGMSG(ZONE_PDD, (L"\tRXDMA_CFG  = 0x%04X\r\n", INREG32(&pPddRegs->RXDMA_CFG)));  // 0040 - DMA receive channels config
        DEBUGMSG(ZONE_PDD, (L"\tTXDMA_CFG  = 0x%04X\r\n", INREG32(&pPddRegs->TXDMA_CFG)));  // 0044 - DMA transmit channels config
//      DEBUGMSG(ZONE_PDD, (L"\tDATA_DMA   = 0x%04X\r\n", INREG32(&pPddRegs->DATA_DMA)));   // 0048 - DMA FIFO data
        DEBUGMSG(ZONE_PDD, (L"\tTXDMA0     = 0x%04X\r\n", INREG32(&pPddRegs->TXDMA0)));     // 0050 - Transmit DMA control 0
        DEBUGMSG(ZONE_PDD, (L"\tTXDMA1     = 0x%04X\r\n", INREG32(&pPddRegs->TXDMA1)));     // 0054 - Transmit DMA control 1
        DEBUGMSG(ZONE_PDD, (L"\tTXDMA2     = 0x%04X\r\n", INREG32(&pPddRegs->TXDMA2)));     // 0058 - Transmit DMA control 2
        DEBUGMSG(ZONE_PDD, (L"\tRXDMA0     = 0x%04X\r\n", INREG32(&pPddRegs->RXDMA0)));     // 0060 - Receive DMA control 0
        DEBUGMSG(ZONE_PDD, (L"\tRXDMA1     = 0x%04X\r\n", INREG32(&pPddRegs->RXDMA1)));     // 0064 - Receive DMA control 0
        DEBUGMSG(ZONE_PDD, (L"\tRXDMA2     = 0x%04X\r\n", INREG32(&pPddRegs->RXDMA2)));     // 0068 - Receive DMA control 0
        DEBUGMSG(ZONE_PDD, (L"\tEP0        = 0x%04X\r\n", INREG32(&pPddRegs->EP0)));        // 0080 - Endpoint 0 configuration
        DEBUGMSG(ZONE_PDD, (L"\tEP_RX[0]   = 0x%04X\r\n", INREG32(&pPddRegs->EP_RX[0])));   // 0084 - Endpoint 1 configuration
        DEBUGMSG(ZONE_PDD, (L"\tEP_RX[1]   = 0x%04X\r\n", INREG32(&pPddRegs->EP_RX[1])));   // 0088 - Endpoint 2 configuration
        DEBUGMSG(ZONE_PDD, (L"\tEP_RX[2]   = 0x%04X\r\n", INREG32(&pPddRegs->EP_RX[2])));   // 008C - Endpoint 3 configuration
        DEBUGMSG(ZONE_PDD, (L"\tEP_RX[3]   = 0x%04X\r\n", INREG32(&pPddRegs->EP_RX[3])));   // 0090 - Endpoint 4 configuration
        DEBUGMSG(ZONE_PDD, (L"\tEP_TX[0]   = 0x%04X\r\n", INREG32(&pPddRegs->EP_TX[0])));   // 00C4 - Endpoint 1 configuration
        DEBUGMSG(ZONE_PDD, (L"\tEP_TX[1]   = 0x%04X\r\n", INREG32(&pPddRegs->EP_TX[1])));   // 00C8 - Endpoint 2 configuration
        DEBUGMSG(ZONE_PDD, (L"\tEP_TX[2]   = 0x%04X\r\n", INREG32(&pPddRegs->EP_TX[2])));   // 00CC - Endpoint 3 configuration
        DEBUGMSG(ZONE_PDD, (L"\tEP_TX[3]   = 0x%04X\r\n", INREG32(&pPddRegs->EP_TX[3])));   // 00D0 - Endpoint 4 configuration
        DEBUGMSG(ZONE_PDD, (L"\r\n"));
    }

#ifdef SHOW_CONFIG_REGISTERS
    {
        PHYSICAL_ADDRESS pa;
        OMAP2420_SYSC1_REGS * pConfRegs;

        pa.QuadPart = OMAP2420_SYSC1_REGS_PA;
        pConfRegs   = (OMAP2420_SYSC1_REGS*)MmMapIoSpace(pa, sizeof(OMAP2420_SYSC1_REGS), FALSE);

        if (pConfRegs == NULL)
        {
            DEBUGMSG(ZONE_PDD, (L"USBD PDD_Dump() - ERROR: Unable to map memory for System Control Registers\r\n"));
        }
        else
        {
            DEBUGMSG(ZONE_PDD, (L"Dump System Control Registers\r\n"));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_REVISION      = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_REVISION)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_SYSCONFIG     = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_SYSCONFIG)));
            DEBUGMSG(ZONE_PDD, (L"\tPADCONF_TV_RREF       = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_PADCONF.ulPADCONF_TV_RREF)));
            DEBUGMSG(ZONE_PDD, (L"\tPADCONF_USB0_RCV      = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_PADCONF.ulPADCONF_USB0_RCV)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_DEBOBS        = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_DEBOBS)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_DEVCONF       = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_DEVCONF)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_EMU_SUPPORT   = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_EMU_SUPPORT)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_MSUSPENDMUX_0 = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_MSUSPENDMUX_0)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_MSUSPENDMUX_1 = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_MSUSPENDMUX_1)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_MSUSPENDMUX_2 = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_MSUSPENDMUX_2)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_MSUSPENDMUX_3 = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_MSUSPENDMUX_3)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_MSUSPENDMUX_4 = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_MSUSPENDMUX_4)));
            DEBUGMSG(ZONE_PDD, (L"\tCONTROL_MSUSPENDMUX_5 = 0x%08X\r\n", INREG32(&pConfRegs->ulCONTROL_MSUSPENDMUX_5)));
            DEBUGMSG(ZONE_PDD, (L"\r\n"));

            // Unmap System Control Registers
            MmUnmapIoSpace((VOID*)pConfRegs, sizeof(OMAP2420_SYSC1_REGS));
            pConfRegs = NULL;
        }
    }
#endif // SHOW_CONFIG_REGISTERS
#ifdef SHOW_OTG_REGISTERS
    {
        PHYSICAL_ADDRESS pa;
        OMAP2420_OTG_REGS * pOtgRegs;

        pa.QuadPart = OMAP2420_OTG_REGS_PA;
        pOtgRegs    = (OMAP2420_OTG_REGS*)MmMapIoSpace(pa, sizeof(OMAP2420_OTG_REGS), FALSE);

        if (pOtgRegs == NULL)
        {
            DEBUGMSG(ZONE_PDD, (L"USBD PDD_Dump() - ERROR: Unable to map memory for USB OTG Registers\r\n"));
        }
        else
        {
            DEBUGMSG(ZONE_PDD, (L"Dump USB OTG registers\r\n"));
            DEBUGMSG(ZONE_PDD, (L"\tREV      = 0x%08X\r\n", INREG32(&pOtgRegs->REV)));
            DEBUGMSG(ZONE_PDD, (L"\tSYSCON_1 = 0x%08X\r\n", INREG32(&pOtgRegs->SYSCON_1)));
            DEBUGMSG(ZONE_PDD, (L"\tSYSCON_2 = 0x%08X\r\n", INREG32(&pOtgRegs->SYSCON_2)));
            DEBUGMSG(ZONE_PDD, (L"\tCTRL     = 0x%08X\r\n", INREG32(&pOtgRegs->CTRL)));
            DEBUGMSG(ZONE_PDD, (L"\tIRQ_EN   = 0x%04X\r\n", INREG32(&pOtgRegs->IRQ_EN)));
            DEBUGMSG(ZONE_PDD, (L"\tIRQ_SRC  = 0x%04X\r\n", INREG32(&pOtgRegs->IRQ_SRC)));
            DEBUGMSG(ZONE_PDD, (L"\tOUTCTRL  = 0x%04X\r\n", INREG32(&pOtgRegs->OUTCTRL)));
            DEBUGMSG(ZONE_PDD, (L"\tTEST     = 0x%04X\r\n", INREG32(&pOtgRegs->TEST)));
            DEBUGMSG(ZONE_PDD, (L"\tVC       = 0x%08X\r\n", INREG32(&pOtgRegs->VC)));
            DEBUGMSG(ZONE_PDD, (L"\r\n"));

            // Unmap USB OTG Registers
            MmUnmapIoSpace((VOID*)pOtgRegs, sizeof(OMAP2420_OTG_REGS));
            pOtgRegs = NULL;
        }
    }
#endif // SHOW_OTG_REGISTERS
#ifdef SHOW_INTC_MPU_REGISTERS
    {
        PHYSICAL_ADDRESS pa;
        OMAP2420_MPUINTC_REGS * pIntcMpuRegs;

        pa.QuadPart  = OMAP2420_INTC_MPU_REGS_PA;
        pIntcMpuRegs = (OMAP2420_MPUINTC_REGS*)MmMapIoSpace(pa, sizeof(OMAP2420_MPUINTC_REGS), FALSE);

        if (pIntcMpuRegs == NULL)
        {
            DEBUGMSG(ZONE_PDD, (L"USBD PDD_Dump() - ERROR: Unable to map memory for INTC MPU Registers\r\n"));
        }
        else
        {
            DEBUGMSG(ZONE_PDD, (L"Dump INTC MPU Registers\r\n"));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_REVISION     = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_REVISION)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_SYSCONFIG    = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_SYSCONFIG)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_SYSSTATUS    = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_SYSSTATUS)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_SIR_IRQ      = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_SIR_IRQ)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_SIR_FIQ      = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_SIR_FIQ)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_CONTROL      = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_CONTROL)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PROTECTION   = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_PROTECTION)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_IDLE         = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_IDLE)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ITR0         = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_ITR0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR0         = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_MIR0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR_CLEAR0   = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_MIR_CLEAR0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR_SET0     = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_MIR_SET0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ISR_SET0     = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_ISR_SET0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ISR_CLEAR0   = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_ISR_CLEAR0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PENDING_IRQ0 = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_PENDING_IRQ0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PENDING_FIQ0 = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_PENDING_FIQ0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ITR1         = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_ITR1)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR1         = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_MIR1)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR_CLEAR1   = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_MIR_CLEAR1)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR_SET1     = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_MIR_SET1)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ISR_SET1     = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_ISR_SET1)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ISR_CLEAR1   = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_ISR_CLEAR1)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PENDING_IRQ1 = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_PENDING_IRQ1)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PENDING_FIQ1 = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_PENDING_FIQ1)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ITR2         = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_ITR2)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR2         = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_MIR2)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR_CLEAR2   = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_MIR_CLEAR2)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR_SET2     = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_MIR_SET2)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ISR_SET2     = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_ISR_SET2)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ISR_CLEAR2   = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_ISR_CLEAR2)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PENDING_IRQ2 = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_PENDING_IRQ2)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PENDING_FIQ2 = 0x%08X\r\n", INREG32(&pIntcMpuRegs->ulINTC_PENDING_FIQ2)));
            DEBUGMSG(ZONE_PDD, (L"\r\n"));
            // Unmap INTC MPU Registers
            MmUnmapIoSpace((VOID*)pIntcMpuRegs, sizeof(OMAP2420_MPUINTC_REGS));
            pIntcMpuRegs = NULL;
        }
    }
#endif // SHOW_INTC_MPU_REGISTERS
#ifdef SHOW_INTC_IVA_REGISTERS
    {
        PHYSICAL_ADDRESS pa;
        OMAP2420_IVAINTC_REGS * pIntcIvaRegs;

        pa.QuadPart  = OMAP2420_INTC_IVA_REGS_PA;
        pIntcIvaRegs = (OMAP2420_IVAINTC_REGS*)MmMapIoSpace(pa, sizeof(OMAP2420_IVAINTC_REGS), FALSE);

        if (pIntcIvaRegs == NULL)
        {
            DEBUGMSG(ZONE_PDD, (L"USBD PDD_Dump() - ERROR: Unable to map memory for INTC IVA Registers\r\n"));
        }
        else
        {
            DEBUGMSG(ZONE_PDD, (L"Dump INTC IVA Registers\r\n"));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_REVISION     = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_REVISION)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_SYSCONFIG    = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_SYSCONFIG)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_SYSSTATUS    = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_SYSSTATUS)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_SIR_IRQ      = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_SIR_IRQ)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_SIR_FIQ      = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_SIR_FIQ)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_CONTROL      = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_CONTROL)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PROTECTION   = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_PROTECTION)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_IDLE         = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_IDLE)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ITR0         = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_ITR0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR0         = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_MIR0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR_CLEAR0   = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_MIR_CLEAR0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_MIR_SET0     = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_MIR_SET0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ISR_SET0     = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_ISR_SET0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_ISR_CLEAR0   = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_ISR_CLEAR0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PENDING_IRQ0 = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_PENDING_IRQ0)));
            DEBUGMSG(ZONE_PDD, (L"\tINTC_PENDING_FIQ0 = 0x%08X\r\n", INREG32(&pIntcIvaRegs->ulINTC_PENDING_FIQ0)));
            DEBUGMSG(ZONE_PDD, (L"\r\n"));

            // Unmap INTC IVA Registers
            MmUnmapIoSpace((VOID*)pIntcIvaRegs, sizeof(OMAP2420_IVAINTC_REGS));
            pIntcIvaRegs = NULL;
        }
    }
#endif // SHOW_INTC_IVA_REGISTERS

    DEBUGMSG(ZONE_PDD, (L"============================================================\r\n"));
}


#define PLATFORM_TYPE_STRING_MAX_NUM_BYTES 48


static void printUsbdDriverSettings()
{
#ifdef DEBUG
    LPTSTR            defaultClientName;
    PLATFORMVERSION * platVer;
    DWORD             numBytesReturned;
    DWORD             infoType;
    DWORD             platformTypeStringMaxBytes = PLATFORM_TYPE_STRING_MAX_NUM_BYTES;
    DWORD             outputBufferSize;
    DWORD             i;
    DWORD             index = 0;
    DWORD             newStringSize      = 0;
    DWORD             numPlatformTypes   = 0;
    wchar_t         * platformTypeString = NULL;
    wchar_t         * platformTypes[(PLATFORM_TYPE_STRING_MAX_NUM_BYTES / sizeof(wchar_t))];


    DEBUGMSG(ZONE_PDD, (L"\r\n"));
    DEBUGMSG(ZONE_PDD, (L"----------------------------------------\r\n"));
    DEBUGMSG(ZONE_PDD, (L"USB Client Driver Settings\r\n"));
    DEBUGMSG(ZONE_PDD, (L"Built on %s at %s\r\n", TEXT(__DATE__), TEXT(__TIME__)));
    DEBUGMSG(ZONE_PDD, (L"\r\n"));


    // Get default client type
    defaultClientName = (LPTSTR)LocalAlloc(LPTR, MAX_LOADSTRING);
    if (defaultClientName)
    {
        if (GetDefaultClientName((LPTSTR)defaultClientName) == ERROR_SUCCESS)
        {
            DEBUGMSG(ZONE_PDD, (L"USBD Default Client = %s\r\n", defaultClientName));
        }
        else
        {
            DEBUGMSG(ZONE_PDD, (L"USBD Default Client = UNKNOWN\r\n"));
        }
        LocalFree((HLOCAL)defaultClientName);
    }
    else
    {
        DEBUGMSG(ZONE_PDD, (L"USBD Default Client = UNKNOWN\r\n"));
    }

    // Get the USB Device's Platform Type Array.
    // 32 bytes should be enough space to store the Platform Type string.
    platformTypeString = (wchar_t *)LocalAlloc(LPTR, platformTypeStringMaxBytes);
    if (platformTypeString)
    {
        infoType = SPI_GETPLATFORMTYPE;

        if (KernelIoControl(IOCTL_HAL_GET_DEVICE_INFO,
                            &infoType,
                            sizeof(infoType),
                            platformTypeString,
                            platformTypeStringMaxBytes,
                            &numBytesReturned))
        {
            for (i = 0; i < (numBytesReturned / sizeof(wchar_t)); i++)
            {
                // Loop through platformTypeString one character at a time until we find a null string terminator
                if (platformTypeString[i] == (wchar_t)'\0')
                {
                    // If (index == i), then we have found two consecutive null string terminators.
                    if (i > index)
                    {
                        newStringSize = (i + 1 - index) * sizeof(wchar_t);

                        // Allocate a new element in platformTypes to contain the string we have found
                        platformTypes[numPlatformTypes] = (wchar_t *)LocalAlloc(LPTR, newStringSize);
                        if (platformTypes[numPlatformTypes])
                        {
                            // Extract the string from platformTypeString
                            // Note: Pass the number of characters (NOT including
                            //       the NULL terminator), not the number of bytes!
                            wcsncpy(platformTypes[numPlatformTypes], (platformTypeString + index), (i - index));
                            numPlatformTypes++;
                            index = i + 1;
                        }
                    }
                }
            }

            // At this point, all of the strings contained in platformTypeString should have
            // been parsed and placed into separate elements within the platformTypes[] array.
            // numPlatformTypes should contain the number of platform types.
        }
        else
        {
            DEBUGMSG(ZONE_PDD, (L"USBD Platform Type String = UNKNOWN\r\n"));
        }

        LocalFree((HLOCAL)platformTypeString);
        platformTypeString = NULL;
    }

    if (numPlatformTypes > 0)
    {
        infoType         = SPI_GETPLATFORMVERSION;
        outputBufferSize = numPlatformTypes * sizeof(PLATFORMVERSION);

        // Get the Windows Mobile Platform Version Array.
        platVer = (PLATFORMVERSION *)LocalAlloc(LPTR, outputBufferSize);
        if (platVer)
        {
            if (KernelIoControl(IOCTL_HAL_GET_DEVICE_INFO,
                                &infoType,
                                sizeof(infoType),
                                platVer,
                                outputBufferSize,
                                &numBytesReturned))
            {
                if (outputBufferSize == numBytesReturned)
                {
                    for (i = 0; i < numPlatformTypes; i++)
                    {
                        DEBUGMSG(ZONE_PDD, (L"USBD Platform: %s (Version %d.%d)\r\n", platformTypes[i], platVer[i].dwMajor, platVer[i].dwMinor));
                    }
                }
            }
            LocalFree((HLOCAL)platVer);
            platVer = NULL;
        }
    }

    // Free the memory allocated for platform version strings.
    for (i = 0; i < numPlatformTypes; i++)
    {
        if (platformTypes[i])
        {
            LocalFree((HLOCAL)platformTypes[i]);
            platformTypes[i] = NULL;
        }
    }

    DEBUGMSG(ZONE_PDD, (L"----------------------------------------\r\n"));
    DEBUGMSG(ZONE_PDD, (L"\r\n"));
#endif
}

//------------------------------------------------------------------------------
