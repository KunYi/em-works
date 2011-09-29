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
//------------------------------------------------------------------------------
//
//  File:  pdd.c
//
//  This file contains USB function PDD implementation. Actual implementation
//  doesn't use DMA transfers and it doesn't support ISO endpoints.
//
#include <windows.h>
#include <omap2420.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <oal.h>

extern void PDD_Dump(DWORD *pPdd);

#pragma optimize ( "g", on )

//------------------------------------------------------------------------------

#ifdef DEBUG
DBGPARAM dpCurSettings;
#endif

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
//  Internal PDD structure which holds info about endpoint direction, max packet
//  size and active transfer.
//
typedef struct {
    WORD maxPacketSize;
    BOOL dirRx;
    BOOL zeroLength;
    STransfer *pTransfer;
} USBFN_EP;

//------------------------------------------------------------------------------
//
//  Type:  USBFN_PDD
//
//  Internal PDD context structure.
//
typedef struct {

    VOID *pMddContext;
    PFN_UFN_MDD_NOTIFY pfnNotify;

    OMAP2420_USBD_REGS *pUSBDRegs;

    HANDLE hClk;

    DWORD devState;
    BOOL selfPowered;

    BOOL setupDirRx;
    WORD setupCount;
    HANDLE hSetupEvent;

    USBFN_EP ep[USBD_EP_COUNT];

    BOOL fakeDsChange;                  // To workaround MDD problem

} USBFN_PDD;

USBFN_PDD g_usbfnpdd;

#ifdef DEBUG

#define ZONE_INTERRUPTS         DEBUGZONE(8)
#define ZONE_POWER              DEBUGZONE(9)
#define ZONE_PDD                DEBUGZONE(15)

#endif

void ConnectHardware();
void DisconnectHardware();

//------------------------------------------------------------------------------
//
//  Function:  Log2
//
//  Trivial log with base 2 function used in EP configuration.
//
static DWORD Log2(DWORD value)
{
    DWORD rc = 0;
    while (value != 0) {
        value >>= 1;
        rc++;
    }
    if (rc > 0) rc--;
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  SetupInterrupt
//
//  This function handles setup packet interrupts.
//
static VOID SetupEvent(USBFN_PDD *pPdd)
{
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    WORD data[4];
    USB_DEVICE_REQUEST *pSetup = (USB_DEVICE_REQUEST*)data;

    do {

        // Select setup FIFO (this clears USBD_INT_SETUP flag)
        OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SETUP);

        // Read setup data
        data[0] = (WORD)INREG32(&pUSBDRegs->DATA);
        data[1] = (WORD)INREG32(&pUSBDRegs->DATA);
        data[2] = (WORD)INREG32(&pUSBDRegs->DATA);
        data[3] = (WORD)INREG32(&pUSBDRegs->DATA);

        // Deselect setup FIFO
        OUTREG32(&pUSBDRegs->EP_NUM, 0);

    } while ((INREG32(&pUSBDRegs->IRQ_SRC) & USBD_INT_SETUP) != 0);

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"USBD SetupEvent() - data[0] = 0x%04X\r\n", data[0]));
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"USBD SetupEvent() - data[1] = 0x%04X\r\n", data[1]));
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"USBD SetupEvent() - data[2] = 0x%04X\r\n", data[2]));
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"USBD SetupEvent() - data[3] = 0x%04X\r\n", data[3]));

    // Save setup packet direction & size for later use
    pPdd->setupDirRx = (pSetup->bmRequestType & 0x80) == 0;
    pPdd->setupCount = pSetup->wLength;

    // MDD doesn't call PDD back on configure message
    if (
        pSetup->bmRequestType == 0 &&
        pSetup->bRequest == USB_REQUEST_SET_CONFIGURATION
    ) {
        if (pSetup->wValue != 0) {
            // Move device to configured state
            OUTREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_DEV_CFG);
            // Set self powered flag
            if (pPdd->selfPowered) {
                SETREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_SELF_PWR);
            } else {
                CLRREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_SELF_PWR);
            }
        } else {
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
    DWORD epNum, stat, space, count, remain;
    UCHAR *pBuffer;
    WORD data;

    // Get active transfer
    pTransfer = pPdd->ep[endPoint].pTransfer;

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"UsbFnPdd!IssueTxTransfer: EP %d pTransfer 0x%x (%d, %d, %d)\r\n",
        endPoint, pTransfer, pTransfer != NULL ? pTransfer->cbBuffer : 0,
        pTransfer != NULL ? pTransfer->cbTransferred : 0,
        pTransfer != NULL ? pTransfer->dwUsbError : -1
    ));

    // Select EP
    epNum = (USBD_EP_NUM & endPoint) | USBD_EP_NUM_DIRIN;
    OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SEL | epNum);

    // Get EP status
    stat = INREG32(&pUSBDRegs->STAT_FLG);

    // Depending on EP status
    if ((stat & USBD_STAT_STALL) != 0) {
        // We issued stall, remove it...
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_HALT);
        // We are done
        goto clean;
    }

    // When transfer is NULL it is handshake ACK
    if (pTransfer == NULL) goto clean;

    // Is this final interrupt of transfer? 
    if (
        pTransfer->cbTransferred == pTransfer->cbBuffer && 
        !pPdd->ep[endPoint].zeroLength
    ) {
        pTransfer->dwUsbError = UFN_NO_ERROR;
        complete = TRUE;
        goto clean;
    }

    pBuffer = (UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
    space = pTransfer->cbBuffer - pTransfer->cbTransferred;

    if (endPoint != 0) {
        // Non Zero Endpoint: No zero length padding needed.
        pPdd->ep[endPoint].zeroLength = FALSE;
    } else {
        // Zero endpoint: Zero length padding needed if last
        // packet is maxPacketSize.
        pPdd->ep[endPoint].zeroLength = (
            space == pPdd->ep[endPoint].maxPacketSize && 
            pPdd->setupCount > pTransfer->cbBuffer
        );
    }        

    // How many bytes we can send just now?
    count = pPdd->ep[endPoint].maxPacketSize;
    if (count > space) count = space;

    // Write data to FIFO
    remain = count;
    while (remain > 1) {
        data = (pBuffer[1] << 8) | pBuffer[0];
        OUTREG16((UINT16*)&pUSBDRegs->DATA, data);
        pBuffer += 2;
        space -= 2;
        remain -= 2;
    }
    if (remain > 0) {
        OUTREG8((UINT8*)&pUSBDRegs->DATA, *pBuffer);
        pBuffer += 1;
        space -= 1;
        remain -= 1;
    }

    // Enable FIFO
    OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_FIFO_EN);

    // We transfered some data
    pTransfer->cbTransferred = pTransfer->cbBuffer - space;

clean:

    // Deselect EP
    OUTREG32(&pUSBDRegs->EP_NUM, epNum);

    // If transaction is complete we should tell MDD
    if (complete) {
        pPdd->ep[endPoint].pTransfer = NULL;
        pPdd->pfnNotify(
            pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer
        );
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
    DWORD epNum, stat, space, count, remain, maxSize;
    UCHAR *pBuffer;
    WORD data;

    // Get active transfer
    pTransfer = pPdd->ep[endPoint].pTransfer;

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"UsbFnPdd!IssueRxTransfer: EP %d pTransfer 0x%x (%d, %d, %d)\r\n",
        endPoint, pTransfer, pTransfer != NULL ? pTransfer->cbBuffer : 0,
        pTransfer != NULL ? pTransfer->cbTransferred : 0,
        pTransfer != NULL ? pTransfer->dwUsbError : -1
    ));

    // Select EP
    epNum = USBD_EP_NUM & endPoint;
    OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SEL | epNum);

    // Get EP status
    stat = INREG32(&pUSBDRegs->STAT_FLG);

    // Depending on EP status
    if ((stat & USBD_STAT_STALL) != 0) {
        // We issued stall, remove it...
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_HALT);
        // We are done
        goto clean;
    }

    // When transfer is NULL it is handshake ACK
    if (pTransfer == NULL) goto clean;

    // Get maxPacketSize
    maxSize = pPdd->ep[endPoint].maxPacketSize;

        pBuffer = (UCHAR*)pTransfer->pvBuffer + pTransfer->cbTransferred;
        space = pTransfer->cbBuffer - pTransfer->cbTransferred;

        // Get EP status flag
        stat = INREG32(&pUSBDRegs->STAT_FLG);

        // Get number of bytes in FIFO
        if ((stat & USBD_STAT_FIFO_EMPTY) != 0) {
            count = 0;
        } else if ((stat & USBD_STAT_FIFO_FULL) != 0) {
            count = maxSize;
        } else {
            count = INREG32(&pUSBDRegs->RXFSTAT) & USBD_RFXSTAT_COUNT;
        }

        // Read data
        remain = count;
        while (remain > 1) {
            data = (WORD)INREG32(&pUSBDRegs->DATA);
            if (space > 1) {
                pBuffer[0] = (UCHAR)data;
                pBuffer[1] = (UCHAR)(data >> 8);
                pBuffer += 2;
                space -= 2;
            }
            remain -= 2;
        }
        if (remain > 0) {
            data = (WORD)INREG32(&pUSBDRegs->DATA);
            if (space > 0) {
                *pBuffer = (UCHAR)data;
                pBuffer += 1;
                space -= 1;
            }
            remain -= 1;
        }

        // We transfered some data
        pTransfer->cbTransferred = pTransfer->cbBuffer - space;

        // Is this end of transfer?
        if (
            pTransfer->cbTransferred == pTransfer->cbBuffer || count < maxSize
        ) {
            // Yes, set return code
            pTransfer->dwUsbError = UFN_NO_ERROR;
            // And complete flag
            complete = TRUE;
        } else {
            // No, enable FIFO for next packet
            OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_FIFO_EN);
        }

clean:

    // Deselect EP
    OUTREG32(&pUSBDRegs->EP_NUM, epNum);

    // If transaction is complete we should tell MDD
    if (complete) {
        pPdd->ep[endPoint].pTransfer = NULL;
        pPdd->pfnNotify(
            pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer
        );
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

    // Get device state & change
    state = INREG32(&pUSBDRegs->DEVSTAT);
    change = state ^ pPdd->devState;

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"UsbFnPdd!DevStatEvent: Device state %x, change %x\r\n",
        state, change
    ));

#ifdef USE_OLD_CABLE_DETECT_METHOD
    // Attach/deattach
    if ((change & USBD_DEVSTAT_ATT) != 0) {
        if ((state & USBD_DEVSTAT_ATT) != 0) {
            // TODO: Call bus driver (OTG?) to move HW from deep sleep
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_FULL_SPEED);
        } else {
            // Clear source bit
            // We are not configured anymore
            OUTREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_CLR_CFG);
            // TODO: Call bus driver (OTG?) to move HW to deep sleep
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
            // Don't process other changes (we are disconnected)
            goto clean;
        }
    }
#endif // USE_OLD_CABLE_DETECT_METHOD

    // Reset
    if ((change & (USBD_DEVSTAT_USB_RESET|USBD_DEVSTAT_DEF)) != 0) {
        if ((state & USBD_DEVSTAT_USB_RESET) == 0) {
            // OTG may not detect attach/detach events correctly on some platforms
            // Simulate a attach/detach event to clear any previous state on reset
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_FULL_SPEED);
            // Tell MDD about reset...
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_RESET);
        }
        // Enable interrupts
        OUTREG32(&pUSBDRegs->IRQ_EN, USBD_IRQ_MASK);
        // In the middle of a reset don't process other changes
        goto clean;
    }

    // Suspend/resume
    if ((change & USBD_DEVSTAT_SUS) != 0) {
        if ((state & USBD_DEVSTAT_SUS) != 0) {
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_SUSPEND);
            // Read device status
            INREG32(&pUSBDRegs->DEVSTAT);
        } else {
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_RESUME);
        }
    }


    // Addressed
    if ((change & USBD_DEVSTAT_ADD) != 0) {
        if ((state & USBD_DEVSTAT_ADD) != 0) {
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SET_ADDRESS, 1);
        } else {
            // Let MDD process change
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_CONFIGURED, 0 );
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SET_ADDRESS,0 );
        }
    }

clean:
    // Save device state for next interrupt
    pPdd->devState = state;
}

//------------------------------------------------------------------------------
//
//  Function:  InterruptThread
//
//  This is interrupt thread. It controls responsed to hardware interrupt. To
//  reduce code length it calls interrupt specific functions.
//
DWORD InterruptThread(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD source, ep;
    while (TRUE) {
        BOOL fInterrupt = FALSE;
    
        // Get interrupt source
        source = INREG32(&pUSBDRegs->IRQ_SRC);

        OALMSG(OAL_ETHER&&OAL_FUNC, (
            L"UsbFnPdd!InterruptThread: Interrupt source %x enabled %x\r\n", source));
        // Device state
        if ((source & USBD_INT_DS_CHG) != 0 || pPdd->fakeDsChange) {
            // Handle device state change
            DevStatEvent(pPdd);
            // Clear fake DsChange flag
            pPdd->fakeDsChange = FALSE;
            if (( source & USBD_INT_DS_CHG) != 0 ) 
                OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_DS_CHG);
   	     fInterrupt = TRUE;
        }

        // EP0 RX interrupt
        if ((source & USBD_INT_EP0_RX) != 0) {
            // Clear source bit
            OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_EP0_RX);
            // Issue next Rx transfer
            IssueRxTransfer(pPdd, 0);
            fInterrupt = TRUE;
        }

        // EP0 TX interrupt
        if ((source & USBD_INT_EP0_TX) != 0) {
            // Clear source bit
            OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_EP0_TX);
            // Issue next Tx transfer
            IssueTxTransfer(pPdd, 0);
            fInterrupt = TRUE;
        }

        // EPn RX interrupt
        if ((source & USBD_INT_EP_RX) != 0) {
            // Get EP number
            ep = (INREG32(&pUSBDRegs->EP_STAT) >> 8) & USBD_EP_NUM;
            // Clear source bit
            OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_EP_RX);
            // Handle event
            IssueRxTransfer(pPdd, ep);
            fInterrupt = TRUE;
        }

        // EPn TX interrupt
        if ((source & USBD_INT_EP_TX) != 0) {
            // Get EP number
            ep = INREG32(&pUSBDRegs->EP_STAT) & USBD_EP_NUM;
            // Clear source bit
            OUTREG32(&pUSBDRegs->IRQ_SRC, USBD_INT_EP_TX);
            // Handle event
            IssueTxTransfer(pPdd, ep);
            fInterrupt = TRUE;
        }

        // Setup Packet
        if ((source & USBD_INT_SETUP) != 0) {
            // Reading setup FIFO clears interrupt
            SetupEvent(pPdd);
            fInterrupt = TRUE;
        }

        if( !fInterrupt )
        {
            break;
        }
    }
   return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------

DWORD WINAPI UfnPdd_IssueTransfer(
    VOID *pPddContext, DWORD endPoint, STransfer *pTransfer
) {
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    // Save transfer for interrupt thread
    pPdd->ep[endPoint].pTransfer = pTransfer;
    // Sync Length with buffer
     if (pTransfer->pvBuffer == NULL) pTransfer->cbBuffer = 0;

    // Depending on direction
    if (TRANSFER_IS_IN(pTransfer)) {
        pPdd->ep[endPoint].zeroLength = (pTransfer->cbBuffer == 0);
        IssueTxTransfer(pPdd, endPoint);
    } else {
        pPdd->ep[endPoint].zeroLength = FALSE;
        // Select EP
        epNum = USBD_EP_NUM & endPoint;
        OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SEL | epNum);
        // Enable EP FIFO
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_FIFO_EN);
        // Deselect EP
        OUTREG32(&pUSBDRegs->EP_NUM, epNum);
    }
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------

DWORD WINAPI UfnPdd_AbortTransfer(
    VOID *pPddContext, DWORD endPoint, STransfer *pTransfer
) {
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    // Select EP
    epNum = USBD_EP_NUM & endPoint;
    if (TRANSFER_IS_IN(pTransfer)) epNum |= USBD_EP_NUM_DIRIN;
    OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SEL | epNum);

    // Clear EP
    OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_EP);

    // Deselect EP
    OUTREG32(&pUSBDRegs->EP_NUM, epNum);

    // Finish transfer
    pPdd->ep[endPoint].pTransfer = NULL;
    pTransfer->dwUsbError = UFN_CANCELED_ERROR;
    pPdd->pfnNotify(
        pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer
    );

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

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"UsbFnPdd_StallEndpoint %d\r\n", endPoint
    ));

    if (endPoint == 0) {
        // Stall next EP0 transaction
       OUTREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_STALL_CMD);
       EdbgOutputDebugString("***********Read USBD->SYSCON_2---=%x\r\n", INREG32(&pUSBDRegs->SYSCON2));                     // 0080 - Endpoint 0 configuration

    } else {

        // Select EP
        epNum = USBD_EP_NUM & endPoint;
        if (!pPdd->ep[endPoint].dirRx) epNum |= USBD_EP_NUM_DIRIN;
        
        OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SEL | epNum);

        // Halt EP
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_SET_HALT);

        // Deselect EP
        OUTREG32(&pUSBDRegs->EP_NUM, epNum);
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

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"UsbFnPdd_ClearEndpoint %d\r\n", endPoint
    ));

    // Endpoint can't be zero
    if (endPoint == 0) goto clean;
    
    // Select EP
    epNum = USBD_EP_NUM & endPoint;
    if (!pPdd->ep[endPoint].dirRx) epNum |= USBD_EP_NUM_DIRIN;

    OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SEL | epNum);

    // Reset endpoint - clear halt isn't sufficient
    OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_RESET_EP);

    // Deselect EP
    OUTREG32(&pUSBDRegs->EP_NUM, epNum);

    // Done
    rc = ERROR_SUCCESS;

clean:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IsEndpointHalted
//
DWORD WINAPI UfnPdd_IsEndpointHalted(
    VOID *pPddContext, DWORD endPoint, BOOL *pHalted
) {
    DWORD rc = ERROR_INVALID_FUNCTION;
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"UsbFnPdd_IsEndpointHalted %d\r\n", endPoint
    ));

    // Endpoint can't be zero
    if (endPoint == 0) goto clean;
    

    // Select EP
    epNum = USBD_EP_NUM & endPoint;
    if (!pPdd->ep[endPoint].dirRx) epNum |= USBD_EP_NUM_DIRIN;

    OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SEL | epNum);

    // Is EP halted?
    *pHalted = (INREG32(&pUSBDRegs->STAT_FLG) & USBD_STAT_HALTED) != 0;

    // Deselect EP
    OUTREG32(&pUSBDRegs->EP_NUM, epNum);

    // Done
    rc = ERROR_SUCCESS;
    
clean:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_InitiateRemoteWakeup
//
//  Send the control status handshake.
//
DWORD WINAPI UfnPdd_SendControlStatusHandshake(
    VOID *pPddContext, DWORD endPoint
) {
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum, stat;

    // Select EP
    epNum = USBD_EP_NUM & endPoint;
    if (pPdd->setupDirRx) epNum |= USBD_EP_NUM_DIRIN;

    OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SEL | epNum);

    // Get actual status
    stat = INREG32(&pUSBDRegs->STAT_FLG);
    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"UsbFnPdd_SendControlStatusHandhake: %d %s (stat %x)\r\n",
        endPoint, pPdd->setupDirRx ? L"IN" : L"OUT", stat
    ));

    // Don't send handshake when EP is stall
    if ((stat & USBD_STAT_STALL) == 0) {
        // Clear & enable FIFO
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_EP);
        OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_FIFO_EN);
    }

    // Deselect EP
    OUTREG32(&pUSBDRegs->EP_NUM, epNum);
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

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"UsbFnPdd_InitiateRemoteWakeup\r\n"));
    SETREG32(&pUSBDRegs->SYSCON2, USBD_SYSCON2_RMT_WKP);
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_PowerDown
//
VOID WINAPI UfnPdd_PowerDown(VOID *pPddContext)
{
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_PowerUp
//
VOID WINAPI UfnPdd_PowerUp(VOID *pPddContext)
{
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IOControl
//
DWORD WINAPI UfnPdd_IOControl(
    VOID *pPddContext, IOCTL_SOURCE source, DWORD code, UCHAR *pInBuffer,
    DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize
) {
    DWORD rc = ERROR_INVALID_PARAMETER;
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    UFN_PDD_INFO *pInfo;

    switch (code) {
    case IOCTL_UFN_GET_PDD_INFO:
        if (source != BUS_IOCTL) break;
        if (pOutBuffer == NULL || outSize < sizeof(UFN_PDD_INFO)) break;
        pInfo = (UFN_PDD_INFO*)pOutBuffer;
        pInfo->InterfaceType = Internal;
        pInfo->BusNumber = 0;
        pInfo->dwAlignment = sizeof(DWORD);
        rc = ERROR_SUCCESS;
        break;
    case IOCTL_BUS_GET_POWER_STATE:
        break;

    case IOCTL_BUS_SET_POWER_STATE:
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

    // Unmap USBD controller registers
    if (pPdd->pUSBDRegs != NULL) {
        pPdd->pUSBDRegs = NULL;
    }

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_DeregisterDevice
//
//  This function is called by MDD to move device to pre-registred state.
//  On OMAP2420 we simply disable all end points.
//
DWORD WINAPI UfnPdd_DeregisterDevice(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD ep;

    // Disable all RX, TX EPs
    OUTREG32(&pUSBDRegs->EP0, 0);
    for (ep = 1; ep < USBD_NONZERO_EP_COUNT; ep++) {
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
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"UfnPdd_Stop\r\n"));

    // Deattach device
    CLRREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_PULLUP_EN);
    DisconnectHardware();

    // Disable USB device PLL clock
    // ClkRelease(pPdd->hClk, 1); @todo

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_DeinitEndpoint
//
//  This function is called when pipe to endpoit is closed. For OMAP2420 we
//  will stop points in UfnPdd_DeregisterDevice.
//
DWORD WINAPI UfnPdd_DeinitEndpoint(VOID *pPddContext, DWORD endPoint)
{
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    OALMSG(OAL_ETHER&&OAL_FUNC, (L"UfnPdd_DeinitEndpoint: %d\r\n", endPoint));

    // Select EP
    epNum = USBD_EP_NUM & endPoint;
    OUTREG32(&pUSBDRegs->EP_NUM, USBD_EP_NUM_SEL | epNum);

    // Clear EP
    OUTREG32(&pUSBDRegs->CTRL, USBD_CTRL_CLR_EP);

    // Deselect EP
    OUTREG32(&pUSBDRegs->EP_NUM, epNum);

    // Done
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_InitEndpoint
//
//  This function is called when pipe to endpoint is created. For OMAP2420
//  all initialization must be done in UfnPdd_RegisterDevice.
//
DWORD WINAPI UfnPdd_InitEndpoint(
    VOID *pContext, DWORD endPoint, UFN_BUS_SPEED speed,
    USB_ENDPOINT_DESCRIPTOR *pEPDesc, VOID *pReserved, UCHAR configValue,
    UCHAR interfaceNumber, UCHAR alternateSetting
) {
    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"UfnPdd_InitEndpoint: %d\r\n", endPoint
    ));
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_SetAddress
//
//  This function is called by MDD when configuration process assigned address
//  to device. For OMAP2420 this is managed by hardware.
//
DWORD WINAPI UfnPdd_SetAddress(VOID *pPddContext, UCHAR address)
{
    OALMSG(OAL_ETHER&&OAL_FUNC, (
        L"UfnPdd_SetAddress: %d\r\n", address
    ));
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
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;

    // Disconnect hardware
    //????DisconnectHardware();

    // Wait for while
    OALStall(10000);

    // Enable interrupts
    OUTREG32(&pUSBDRegs->IRQ_EN, USBD_IRQ_MASK);

    // Attach device to bus (it has no effect when OTG controller is used)
    SETREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_PULLUP_EN|USBD_SYSCON1_SELF_PWR);
	
    //?????ConnectHardware();

    // Set fake device change flag which on first interrupt force
    // device state change handler even if it isn't indicated by hardware
    pPdd->fakeDsChange = TRUE;

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_RegisterDevice
//
//  This function is called by MDD after device configuration was sucessfully
//  verified by UfnPdd_IsEndpointSupportable and
//  UfnPdd_IsConfigurationSupportable. It should initialize hardware for given
//  configuration. Depending on hardware endpoints can be initialized later in
//  UfnPdd_InitEndpoint. For OMAP2420 it isn't a case, so we should do all
//  initialization there.
//
DWORD WINAPI UfnPdd_RegisterDevice(
    VOID *pPddContext, const USB_DEVICE_DESCRIPTOR *pHighSpeedDeviceDesc,
    const UFN_CONFIGURATION *pHighSpeedConfig,
    const USB_CONFIGURATION_DESCRIPTOR *pHighSpeedConfigDesc,
    const USB_DEVICE_DESCRIPTOR *pFullSpeedDeviceDesc,
    const UFN_CONFIGURATION *pFullSpeedConfig,
    const USB_CONFIGURATION_DESCRIPTOR *pFullSpeedConfigDesc,
    const UFN_STRING_SET *pStringSets, DWORD stringSets
) {
    DWORD rc = ERROR_INVALID_PARAMETER;
    USBFN_PDD *pPdd = pPddContext;
    OMAP2420_USBD_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT *pEP;
    DWORD offset, ep, cfg;
    DWORD ifc, epx;


    // Remember self powered flag
    pPdd->selfPowered = (pFullSpeedConfig->Descriptor.bmAttributes & 0x20) != 0;

    // Unlock configuration
    CLRREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_CFG_LOCK);

    // Configure EP0
    offset = 8;
    cfg  = Log2(pFullSpeedDeviceDesc->bMaxPacketSize0 >> 3) << 12;
    cfg |= offset >> 3;
    OUTREG32(&pUSBDRegs->EP0, cfg);
    pPdd->ep[0].maxPacketSize = pFullSpeedDeviceDesc->bMaxPacketSize0;
    offset += pFullSpeedDeviceDesc->bMaxPacketSize0;

    // Configure Rx EPs
    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++) {
        // For each endpoint in interface
        pIFC = &pFullSpeedConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++) {
            pEP = &pIFC->pEndpoints[epx];
            // If it is Tx EP skip it
            if ((pEP->Descriptor.bEndpointAddress & 0x80) != 0) continue;
            // Get EP address
            ep = pEP->Descriptor.bEndpointAddress & 0x0F;
            // Save max packet size & direction
            pPdd->ep[ep].maxPacketSize = pEP->Descriptor.wMaxPacketSize;
            pPdd->ep[ep].dirRx = TRUE;
            // Create EP config
            cfg  = USBD_EP_VALID;
            cfg |= Log2(pEP->Descriptor.wMaxPacketSize >> 3) << 12;
            if ((pEP->Descriptor.bmAttributes & 0x03) == 0x01) {
                cfg |= USBD_EP_ISO;
            }
            cfg |= offset >> 3;
            
            if (ep > 0)
            OUTREG32(&pUSBDRegs->EP_RX[ep - 1], cfg);
            else
                OALMSG(1, (L"+USBFN:: UfnPdd_RegisterDevice EP_RX = %d\r\n", ep - 1));
            
            // Update offset
            offset += pEP->Descriptor.wMaxPacketSize;
        }
    }

    // Configure Tx EPs
    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++) {
        // For each endpoint in interface
        pIFC = &pFullSpeedConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++) {
            pEP = &pIFC->pEndpoints[epx];
            // If it is Rx EP skip it
            if ((pEP->Descriptor.bEndpointAddress & 0x80) == 0) continue;
            // Get EP address
            ep = pEP->Descriptor.bEndpointAddress & 0x0F;
            // Save max packet size & direction
            pPdd->ep[ep].maxPacketSize = pEP->Descriptor.wMaxPacketSize;
            pPdd->ep[ep].dirRx = FALSE;
            // Create EP config
            cfg  = USBD_EP_VALID;
            cfg |= Log2(pEP->Descriptor.wMaxPacketSize >> 3) << 12;
            if ((pEP->Descriptor.bmAttributes & 0x03) == 0x01) {
                cfg |= USBD_EP_ISO;
            }
            cfg |= offset >> 3;
            
            if (ep > 0)
                OUTREG32(&pUSBDRegs->EP_TX[ep - 1], cfg);
            else
                OALMSG(1, (L"+USBFN:: UfnPdd_RegisterDevice EP_TX = %d\r\n", ep - 1));
            // Update offset
            offset += pEP->Descriptor.wMaxPacketSize;
        }
    }

    // Lock configuration
    SETREG32(&pUSBDRegs->SYSCON1, USBD_SYSCON1_CFG_LOCK);

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
DWORD WINAPI UfnPdd_IsEndpointSupportable(
    VOID *pPddContext, DWORD endPoint, UFN_BUS_SPEED speed,
    USB_ENDPOINT_DESCRIPTOR *pEPDesc, UCHAR configurationValue,
    UCHAR interfaceNumber, UCHAR alternateSetting
) {
    USBFN_PDD *pPdd = pPddContext;

    // Update maximal packet size for EP0
    if (endPoint == 0) {
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
//  For OMAP2420 we should check if total descriptor size is smaller
/// than 2040 bytes and round EP sizes. Unfortunately we don't get information
//  about EP0 max packet size. So we will assume maximal 64 byte size.
//
DWORD WINAPI UfnPdd_IsConfigurationSupportable(
    VOID *pPddContext, UFN_BUS_SPEED speed, UFN_CONFIGURATION *pConfig
) {
    DWORD rc = ERROR_INVALID_PARAMETER;
    USBFN_PDD *pPdd = pPddContext;
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT *pEP;
    WORD ifc, epx, count;
    WORD offset, size;


    // TODO: Update self power bit & maxPower

    // We must start with offset 8 + 64 (config plus EP0 size)
    offset = 8 + 64;
    // Clear number of end points
    count = 0;

    // For each interface in configuration
    for (ifc = 0; ifc < pConfig->Descriptor.bNumInterfaces; ifc++) {
        // For each endpoint in interface
        pIFC = &pConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++) {
            pEP = &pIFC->pEndpoints[epx];
            // We support maximal sizes 8, 16, 32 and 64 bytes for non-ISO
            size = pEP->Descriptor.wMaxPacketSize;
            // First round size to supported sizes
            size = 1 << Log2(size);
            // Is it ISO end point?
            if ((pEP->Descriptor.bmAttributes & 0x03) != 0x01) {
                // Non-ISO, max size is 64 bytes
                if (size > 64) size = 64;
            } else {
                // ISO edpoint, maximal size is 512 bytes
                if (size > 512) size = 512;
            }
            // Update EP size
            pEP->Descriptor.wMaxPacketSize = size;
            // Calculate total buffer size
            offset += size;
        }
        // Add number of end points to total count
        count += pIFC->Descriptor.bNumEndpoints;
    }

    // Can we support this configuration?
    if (count < USBD_EP_COUNT && offset <= 2048) rc = ERROR_SUCCESS;

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
DWORD WINAPI UfnPdd_Init(
    LPCTSTR szActiveKey, VOID *pMddContext, UFN_MDD_INTERFACE_INFO *pMddIfc,
    UFN_PDD_INTERFACE_INFO *pPddIfc) 
{
    DWORD rc;
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"+USBFN:: PDD Init\r\n"));
    {
        USBFN_PDD *pPdd;
        OMAP2420_USBD_REGS *pUSBDRegs;
        DWORD ep;
    
        rc = ERROR_INVALID_PARAMETER;
    
        // Allocate and initialize the OHCD object.
        pPdd = &g_usbfnpdd;

        if (pPdd == NULL) goto clean;
    
        // Clear the allocated object.
        memset(pPdd, 0, sizeof(USBFN_PDD));
    
        // Map the USB OHCI registers
        pUSBDRegs = (OMAP2420_USBD_REGS*)OALPAtoUA( OMAP2420_USBD_REGS_PA );
    
        if (pUSBDRegs == NULL) {
            OALMSG(OAL_ERROR, (
                L"ERROR: UfnPdd_Init: Controller registers mapping failed\r\n"
            ));
            goto clean;
        }
        
        pPdd->pUSBDRegs = pUSBDRegs;
    
        // Clear USB Interrupt enable registers
        OUTREG32(&pUSBDRegs->IRQ_EN, 0x0000);
        OUTREG32(&pUSBDRegs->DMA_IRQ_EN, 0x0000);
    
        // Reset all interrupts
        OUTREG32(&pUSBDRegs->IRQ_SRC, 0xFFFFFFFF);
    
        // Disable all RX, TX EPs
        OUTREG32(&pUSBDRegs->EP0, 0);
        for (ep = 0; ep < USBD_NONZERO_EP_COUNT; ep++) {
            OUTREG32(&pUSBDRegs->EP_RX[ep], 0);
            OUTREG32(&pUSBDRegs->EP_TX[ep], 0);
        }
    
        //OUTREG32(&pUSBDRegs->SYSCON1, 0);
    
        // Set PDD interface
        pPddIfc->dwVersion = UFN_PDD_INTERFACE_VERSION;
        pPddIfc->dwCapabilities = UFN_PDD_CAPS_SUPPORTS_FULL_SPEED;
        pPddIfc->dwEndpointCount = USBD_EP_COUNT;
        pPddIfc->pvPddContext = pPdd;
        pPddIfc->pfnDeinit = UfnPdd_Deinit;
        pPddIfc->pfnIsConfigurationSupportable = UfnPdd_IsConfigurationSupportable;
        pPddIfc->pfnIsEndpointSupportable = UfnPdd_IsEndpointSupportable;
        pPddIfc->pfnInitEndpoint = UfnPdd_InitEndpoint;
        pPddIfc->pfnRegisterDevice = UfnPdd_RegisterDevice;
        pPddIfc->pfnDeregisterDevice = UfnPdd_DeregisterDevice;
        pPddIfc->pfnStart = UfnPdd_Start;
        pPddIfc->pfnStop = UfnPdd_Stop;
        pPddIfc->pfnIssueTransfer = UfnPdd_IssueTransfer;
        pPddIfc->pfnAbortTransfer = UfnPdd_AbortTransfer;
        pPddIfc->pfnDeinitEndpoint = UfnPdd_DeinitEndpoint;
        pPddIfc->pfnStallEndpoint = UfnPdd_StallEndpoint;
        pPddIfc->pfnClearEndpointStall = UfnPdd_ClearEndpointStall;
        pPddIfc->pfnSendControlStatusHandshake = UfnPdd_SendControlStatusHandshake;
        pPddIfc->pfnSetAddress = UfnPdd_SetAddress;
        pPddIfc->pfnIsEndpointHalted = UfnPdd_IsEndpointHalted;
        pPddIfc->pfnInitiateRemoteWakeup = UfnPdd_InitiateRemoteWakeup;
        pPddIfc->pfnPowerDown = UfnPdd_PowerDown;
        pPddIfc->pfnPowerUp = UfnPdd_PowerUp;
        pPddIfc->pfnIOControl = UfnPdd_IOControl;
    
        // Save MDD context & notify function
        pPdd->pMddContext = pMddContext;
        pPdd->pfnNotify = pMddIfc->pfnNotify;
    
        // Done
        rc = ERROR_SUCCESS;

    }

clean:
    OALMSG(OAL_ETHER&&OAL_FUNC, (L"-USBFN:: PDD Init\r\n"));
    return rc;
}

//------------------------------------------------------------------------------

extern BOOL UfnPdd_DllEntry(
    HANDLE hDllHandle, DWORD reason, VOID *pReserved
) {
    return TRUE;
}

//------------------------------------------------------------------------------

#pragma optimize ( "", on )
