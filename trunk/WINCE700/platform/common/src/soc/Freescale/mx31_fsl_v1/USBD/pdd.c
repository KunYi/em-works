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
//------------------------------------------------------------------------------
//
// Copyright (C) 2005-2007, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  pdd.c
//
//  This file contains USB function PDD implementation. Actual implementation
//  doesn't use DMA transfers and it doesn't support ISO endpoints.
//
//#pragma optimize( "", off )

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include <celog.h>
#ifdef USB_ELEGANT_INTR
#include <oal.h>
#endif
#pragma warning(pop)

#include "csp.h"
#include "usbd.h"

#include <mx31_usbname.h>
#include <mx31_usbcommon.h>
#include <mx31_usbfnioctl.h>


#define USB_TIMEOUT 10000
#define IDLE_TIMEOUT 3000
#define UFN_TO2 0x20
#define USBFN_TEST_MODE_SUPPORT

#define MTP_SUPPORT

#undef UFNPDD_PWR
#define UFNPDD_PWR 0

#define DEBUG_LOG_USBCV 0

// Remove-W4: Warning C4053 workaround
// The following definition public\common\oak\inc\usbfn.h caused
// warning C4053: one void operand for '?:', so we just use #pragma
// to get rid of those MSFT stuff.
//
// If ZONE_CELOG is set, the debug messages are sent to celog instead of the
// standard debug output.
// This is useful if the debug messages cause timing problems.
/*
#undef DEBUGMSG
#define DEBUGMSG(cond, printf_exp) \
    ( (void)(((cond) ? \
        ( (ZONE_CELOG) ? (CeLogMsg printf_exp), 1 : (NKDbgPrintfW printf_exp), 1) : 0) ) )
*/

#pragma warning(disable: 4053)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
#ifdef DEBUG

#define ZONE_ERROR              DEBUGZONE(0)
#define ZONE_WARNING            DEBUGZONE(1)
#define ZONE_TRANSFER           DEBUGZONE(3)
#define ZONE_INTERRUPTS         DEBUGZONE(8)
#define ZONE_POWER              DEBUGZONE(9)
#define ZONE_FUNCTION           DEBUGZONE(12)
#define ZONE_PDD                DEBUGZONE(15)

extern DBGPARAM dpCurSettings = {
    L"UsbFn", {
            L"Error",       L"Warning",     L"<unused>",     L"<unused>",
            L"<unused>",    L"<unused>",    L"<unused>",     L"<unused>",
            L"Interrupts",  L"Power",       L"<unused>",     L"<unused>",
            L"Function",    L"<unused>",    L"<unused>",     L"PDD"
    },
    0x003 // Default to ZONE_ERROR | ZONE_WARNING
};

#endif

CRITICAL_SECTION g_csRegister;
BOOL g_isInit = FALSE;

#define UFN_MORE_DATA       UFN_NOT_COMPLETE_ERROR

#define LOCK() EnterCriticalSection(&g_csRegister)
#define UNLOCK() LeaveCriticalSection(&g_csRegister)


extern DWORD GetDeviceRegistryParams(
                                     LPCWSTR context, VOID *pBase, DWORD count,
                                     const DEVICE_REGISTRY_PARAM params[]
) ;
extern void SetULPIToClientMode(CSP_USB_REGS *regs);
//extern void SetPHYPowerMgmt(CSP_USB_REGS* pRegs, BOOL fSuspend);
static  BOOL SetDeviceTestMode( USBFN_PDD *pPdd, int iTestMode );

static void PrimeQh0ForZeroTransfer(USBFN_PDD *pPdd, BOOL bIN );
static void FlushEndpoint( USBFN_PDD *pPdd, DWORD epNum, BOOL bIsReceive );


#define TDVirtual(td) (PUSBD_dTD_R_T)DescriptorVirtual(pPdd,(((DWORD)td)<<5))
#define DataVirtual(bp) (UCHAR *)DescriptorVirtual(pPdd,(((DWORD)bp)<<12))

DWORD WINAPI UfnPdd_Start(VOID *pPddContext);
DWORD WINAPI UfnPdd_IssueTransfer(
                                  VOID *pPddContext, DWORD endPoint,
                                  STransfer *pTransfer
                                 );
DWORD WINAPI UfnPdd_StallEndpoint(VOID *pPddContext, DWORD endPoint);

PHYSICAL_ADDRESS phyAddr_TD1;
PHYSICAL_ADDRESS phyAddr_TD2;

PVOID Buffer_Td1;
PVOID Buffer_Td2;

DMA_ADAPTER_OBJECT Adapter1;
DMA_ADAPTER_OBJECT Adapter2;

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
        L"Irq", PARAM_DWORD, TRUE, offset(USBFN_PDD, irq),
            fieldsize(USBFN_PDD, irq), NULL
        }, {
            L"Priority256", PARAM_DWORD, FALSE, offset(USBFN_PDD, priority256),
                fieldsize(USBFN_PDD, priority256), (void *)100
        }, {
            L"OTGSupport", PARAM_DWORD, FALSE, offset(USBFN_PDD, IsOTGSupport),
                fieldsize(USBFN_PDD, IsOTGSupport), (void *)0
        }, {
            L"OTGGroup", PARAM_STRING, TRUE, offset(USBFN_PDD, szOTGGroup),
                fieldsize(USBFN_PDD, szOTGGroup), NULL
        }

};

//------------------------------------------------------------------------------
//
//  Function:  Log2
//
//  Trivial log with base 2 function used in EP configuration.
//
static WORD Log2(WORD value)
{
    WORD rc = 0;
    while (value != 0)
    {
        value >>= 1;
        rc++;
    }
    if (rc > 0)
        rc--;

    return rc;
}

//-------------------------------------------------------------
//
//  Function: DescriptorPhy
//
//  This function is to calculate the descriptor in physical address
//
//  Parameters:
//       pPdd - Pointer to the USBFN_PDD structure
//       vaddr - virtual address of the descriptor
//
//  Return:
//       Physical address of descriptor
//
//--------------------------------------------------------------
DWORD DescriptorPhy(USBFN_PDD *pPdd, DWORD vaddr)
{
    DWORD r=vaddr-(DWORD)(pPdd->qhbuffer)+pPdd->qhbuf_phy;
    return r;
}

//-------------------------------------------------------------
//
//  Function: DescriptorVirtual
//
//  This function is to calculate the descriptor in virtual address
//
//  Parameters:
//       pPdd - Pointer to the USBFN_PDD structure
//       paddr - physical address of the descriptor
//
//  Return:
//       Virtual address of descriptor
//
//--------------------------------------------------------------
DWORD DescriptorVirtual(USBFN_PDD *pPdd, DWORD paddr)
{
    DWORD r=paddr-pPdd->qhbuf_phy+(DWORD)(pPdd->qhbuffer);
    return r;
}

//-------------------------------------------------------------
//
//  Function: ClearUsbstsUI
//
//  This function is used to clear USBSTS.UI
//
//  Parameters:
//      pUSBDRegs - pointer to usb memory map
//--------------------------------------------------------------
void ClearUsbstsUI(CSP_USB_REGS* pUSBDRegs)
{
    USB_USBSTS_T usbSts;
    DWORD* ltemp = (DWORD*)&usbSts;

    RETAILMSG(DEBUG_LOG_USBCV, (L"UI clear\r\n"));
    *ltemp = INREG32(&pUSBDRegs->OTG.USBSTS);
    if (usbSts.UI)
    {
        RETAILMSG(DEBUG_LOG_USBCV, (L"UI is set internally %x\r\n", *ltemp));
        *ltemp &= ~USBD_IRQ_MASK;
        usbSts.UI = 1;
        OUTREG32(&pUSBDRegs->OTG.USBSTS, *ltemp);
    }
}

//-------------------------------------------------------------
//
//  Function: GetSetupPacket
//
//  This function is to retrieve the setup packet data
//
//  Parameters:
//       [IN]  pPdd - Pointer to structure USBFN_PDD
//       [OUT] data - Pointer to an array of DWORD
//
//  Return:
//       The setup packet would be copied to parameter "data"
//
//--------------------------------------------------------------

static void GetSetupPacket(USBFN_PDD *pPdd, PDWORD data)
{
    USB_USBCMD_T cmd;
    DWORD *temp=(DWORD *)&cmd;
    *temp=0;

    // indicate we've read out this setup
    OUTREG32(&pPdd->pUSBDRegs->OTG.ENDPTSETUPSTAT, 1);

    for (;;)
    {

        //RETAILMSG(1, (TEXT("Setup packet\r\n")));
        // Write '1' to Setup Tripwire (SUTW) in USBCMD register.
        *temp=INREG32(&pPdd->pUSBDRegs->OTG.USBCMD);
        cmd.SUTW=1;
        OUTREG32(&pPdd->pUSBDRegs->OTG.USBCMD, *temp);

        //Duplicate contents of dQH.SetupBuffer into local software byte array.
        data[0] = (DWORD)pPdd->qhbuffer->qh[0].SB0;
        data[1] = (DWORD)pPdd->qhbuffer->qh[0].SB1;

        pPdd->qhbuffer->qh[0].SB0=0;
        pPdd->qhbuffer->qh[0].SB1=0;

        DEBUGMSG(ZONE_PDD, (TEXT("GetSetupPacket %8.8lx%8.8lx\r\n"),data[0],data[1] ));
        RETAILMSG(DEBUG_LOG_USBCV, (L"Setup %x %x\r\n", data[0], data[1]));
        *temp=INREG32(&pPdd->pUSBDRegs->OTG.USBCMD);

        // Check whether a new setup was received while we were retrieving
        // the setup, and retrieve again if so.
        if (cmd.SUTW)
        {
            break;
        }
    }

    *temp=0; cmd.SUTW=1;
    CLRREG32(&pPdd->pUSBDRegs->OTG.USBCMD, *temp);

    // Doc claims ENDPTCOMPLETE is set both for Setup and OUT transactions,
    // so it can be normal to have to clear complete now.
#ifdef USBCV_FIX
    // Seems The spec has error, setup event will not change ENDPTCOMPLETE
    // so we do nothing here
#else
    if (INREG32(&(pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE)) & 0x1)
    {
        OUTREG32(&(pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE), 1);
    }
#endif

    if ( pPdd->ep[0]->pTransfer )
    {
        DEBUGMSG(ZONE_WARNING, (TEXT("####### GetSetupPacket: Transfer was ")
                                TEXT("in progress on EP0 (EPSTAT 0x%x, ")
                                TEXT("status 0x%x)\r\n"),
                                INREG32(&(pPdd->pUSBDRegs->OTG.ENDPTSTATUS)),
                                pPdd->qhbuffer->qh[0].dtd.status) );

        // That transfer should be aborted.  If EPSTAT was set then it
        // needs also to be flushed.
        RETAILMSG(DEBUG_LOG_USBCV, (L"Abort?\r\n"));
    }

    if ( INREG32(&(pPdd->pUSBDRegs->OTG.ENDPTSTATUS)) & 0x1)
    {
        DEBUGMSG(ZONE_WARNING, (TEXT("####### GetSetupPacket: Flushing EP 0 ")
                                TEXT("(EPSTATUS 0x%x, QH0:0x%x)\r\n"),
                                INREG32(&(pPdd->pUSBDRegs->OTG.ENDPTSTATUS)),
                                pPdd->qhbuffer->qh[0].dtd.status) );
        RETAILMSG(DEBUG_LOG_USBCV, (L"Flush?\r\n"));

        OUTREG32(&pPdd->pUSBDRegs->OTG.ENDPTFLUSH,1);
        /* Software note: this operation may take a large amount of time
         * depending on the USB bus activity. It is not desirable to have
         * this wait loop within an interrupt service routine.
         *
         * This flush should not normally happen.  Setup only comes while
         * an endpoint is primed, under protocol failure conditions.
         * Otherwise OUT and IN always complete normally since it is the
         * host which sends SETUP and host which drives the IN/OUT protocol
         * on the control ep.
         */

        for (;;)
        {
            if ( (INREG32(&pPdd->pUSBDRegs->OTG.ENDPTFLUSH) & 1) == 0 )
            {
                break;
            }
        }
    }
}

//-------------------------------------------------------------
//
//  Function: InitQh0
//
//  This is used to initialise for EP0, where many transactions are set-up
//  transactions. Where actual data is required, this QH is initialised
//  instead through the normal IssueTransfer method.
//
//  Parameters:
//      pPdd - Pointer to USBFN_PDD
//      len - no of bytes to transfer
//      pTransfer - Pointer to STransfer
//
//  Return: NULL
//
//------------------------------------------------------------------
static void
InitQh0(USBFN_PDD *pPdd, DWORD len, STransfer *pTransfer)
{
    volatile CSP_USB_REG * pOtgReg=&(pPdd->pUSBDRegs->OTG);
    int retry = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pTransfer);

    LOCK();

    pPdd->ep[0]->pTransfer = NULL;
    pPdd->ep[0]->pMappedBufPtr = 0;
    pPdd->ep[0]->dwNextPageIdx = 0;
    pPdd->ep[0]->dwNumPages = 0;
    pPdd->ep[0]->bPagesLocked = FALSE;

    pPdd->ep[USBD_EP_COUNT-1]->pTransfer = NULL;
    pPdd->ep[USBD_EP_COUNT-1]->pMappedBufPtr = 0;
    pPdd->ep[USBD_EP_COUNT-1]->dwNextPageIdx = 0;
    pPdd->ep[USBD_EP_COUNT-1]->dwNumPages = 0;
    pPdd->ep[USBD_EP_COUNT-1]->bPagesLocked = FALSE;

    memset(&pPdd->qhbuffer->qh[0], 0, sizeof(USBD_dQH_T));
    memset(&pPdd->qhbuffer->qh[1], 0, sizeof(USBD_dQH_T));
    memset(&pPdd->qhbuffer->td[0], 0, sizeof(USBD_dTD_R_T));
    memset(&pPdd->qhbuffer->td[1], 0, sizeof(USBD_dTD_R_T));
    pPdd->qhbuffer->bPrimed[0] = FALSE;
    pPdd->qhbuffer->bPrimed[USBD_EP_COUNT-1] = FALSE;


    DEBUGMSG(ZONE_PDD,(L"Initqh0, %x", pPdd->qhbuffer->qh[0].dtd.status));
again:
    if ((pPdd->qhbuffer->qh[0].SB0||pPdd->qhbuffer->qh[0].SB1) && (retry))
    {
        DWORD data[2];

        DEBUGMSG(ZONE_WARNING, (_T("Setup not processed (SETUPSTAT=0x%x)??? ")
                                _T("%x, %x\r\n"),
                                INREG32(&pOtgReg->ENDPTSETUPSTAT),
                                pPdd->qhbuffer->qh[0].SB0,
                                pPdd->qhbuffer->qh[0].SB1));

        RETAILMSG(DEBUG_LOG_USBCV, (L"Update Setup 1\r\n"));

        GetSetupPacket(pPdd, data);

        //OUTREG32(&pOtgReg->ENDPTSETUPSTAT, INREG32(&pOtgReg->ENDPTSETUPSTAT));
        DEBUGMSG(ZONE_WARNING, (L"New setup received when priming=%x, %x\r\n",
                                data[0], data[1]));

        DEBUGCHK ( pPdd->ep[0]->bPagesLocked == 0 );

        UNLOCK();
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SETUP_PACKET, (DWORD)data);
        LOCK();
    }

    memset(&(pPdd->qhbuffer->qh[0]),0, sizeof(USBD_dQH_T));
    pPdd->qhbuffer->bPrimed[0] = FALSE;

    // Endpoint 0
    pPdd->qhbuffer->qh[0].ios=1;
    pPdd->qhbuffer->qh[0].mpl=USB_FULL_HIGH_SPEED_CONTROL_MAX_PACKET_SIZE;
    pPdd->qhbuffer->qh[0].zlt=1;

    //overlayer  default
    {
        DWORD t;
        t=DescriptorPhy(pPdd, (DWORD)&(pPdd->qhbuffer->td[0]));
        t>>=5;
        pPdd->qhbuffer->qh[0].dtd.next_dtd=t;
    }

    OUTREG32(&pOtgReg->ENDPTSETUPSTAT, 0xffff);

    memset (&(pPdd->qhbuffer->td[0]), 0, sizeof(USBD_dTD_T));
    pPdd->qhbuffer->td[0].T=1;
    pPdd->qhbuffer->td[0].next_dtd=0xDEAD;
    pPdd->qhbuffer->td[0].status=0x80;            // Active
    pPdd->qhbuffer->td[0].ioc=1;
    pPdd->qhbuffer->td[0].tb=len;

    OUTREG32(&pOtgReg->ENDPTPRIME, 0x1);  // prime Out transition
    while(INREG32(&pOtgReg->ENDPTPRIME) & 0x1)
    {
        ;
    }

    if ((INREG32(&pOtgReg->ENDPTSTATUS)&1)==0)
    {
        DWORD data[2];

        DEBUGMSG(ZONE_WARNING,(L"PRIME failed on EP0\r\n") );

        if ( INREG32(&pOtgReg->ENDPTSETUPSTAT) & 1 )
        {
            RETAILMSG(DEBUG_LOG_USBCV, (L"Update Setup 2\r\n"));
            GetSetupPacket(pPdd, data);

            DEBUGMSG(ZONE_PDD, (L"New setup received when priming=%x, %x\r\n",
                                data[0], data[1]));

            DEBUGCHK ( pPdd->ep[0]->bPagesLocked == 0 );

            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SETUP_PACKET,
                            (DWORD)data);

            DEBUGMSG(ZONE_PDD, (L"Status for td0=%x, tb=%x\r\n",
                                pPdd->qhbuffer->qh[0].dtd.status,
                                pPdd->qhbuffer->qh[0].dtd.tb));
        }

        retry = 1;
        goto again;
    }

    pPdd->qhbuffer->bPrimed[0] = TRUE;

    UNLOCK();
}

//-------------------------------------------------------------
//
//  Function: PrimeQh0ForZeroTransfer
//
//  This is to prime the end point 0 and is used during the handshake status
//
//  Parameters:
//      pPdd - Pointer to the USBFN_PDD
//      bIN  - TRUE : IN transfer
//             FALSE: OUT transfer
//
//  Return: NULL
//
//------------------------------------------------------------------
static void
PrimeQh0ForZeroTransfer(USBFN_PDD *pPdd, BOOL bIN )
{
    volatile CSP_USB_REG * pOtgReg=&(pPdd->pUSBDRegs->OTG);
    int endp;
    USB_ENDPTSTAT_T stat;
    DWORD *pStat = (DWORD*)&stat;
    USB_ENDPTPRIME_T edptprime;
    DWORD *pPrime=(DWORD * )&edptprime;

    *pPrime = 0;

    if ( bIN )
    {
        edptprime.PETB = 1;
        endp = 1;
    }
    else
    {
        edptprime.PERB = 1;
        endp = 0;
    }

    CeLogMsg(_T("PrimeFZLT: %s"), bIN ? _T("IN") : _T("OUT"));

    *pStat = INREG32(&pOtgReg->ENDPTSTATUS);
    if ( (bIN && (stat.ETBR & 1)) || (!bIN && (stat.ERBR & 1)) )
    {
        DEBUGMSG(ZONE_ERROR, (_T("Can not prime for ZLT (%s): busy\r\n"),
                              bIN ? _T("IN") : _T("OUT")) );
        RETAILMSG(FALSE, (L"Busy\r\n"));
        return;
    }

    memset(&(pPdd->qhbuffer->qh[endp]),0, sizeof(USBD_dQH_T)-6*sizeof(DWORD));
    pPdd->qhbuffer->bPrimed[endp] = FALSE;

    // Endpoint 0
    pPdd->qhbuffer->qh[endp].ios= (endp == 0 ? 1 : 0);
    pPdd->qhbuffer->qh[endp].mpl=USB_FULL_HIGH_SPEED_CONTROL_MAX_PACKET_SIZE;
    pPdd->qhbuffer->qh[endp].zlt=1;
    //overlayer  default
    {
        DWORD t;
        t=DescriptorPhy(pPdd, (DWORD)&(pPdd->qhbuffer->td[endp]));
        t>>=5;
        pPdd->qhbuffer->qh[endp].dtd.next_dtd=t;
        pPdd->qhbuffer->qh[endp].dtd.T=0;
    }

    // OUTREG32(&pOtgReg->ENDPTSETUPSTAT, 0xffff);

    memset (&(pPdd->qhbuffer->td[endp]), 0, sizeof(USBD_dTD_T));
    pPdd->qhbuffer->td[endp].T=1;
    pPdd->qhbuffer->td[endp].next_dtd=0xDEAD;
    pPdd->qhbuffer->td[endp].status=0x80;            // Active
    pPdd->qhbuffer->td[endp].ioc=1;
    pPdd->qhbuffer->td[endp].tb=0;
    //  leave all bp null for a null transfer

    OUTREG32(&pOtgReg->ENDPTPRIME, *pPrime);  // prime Out transition
    while(INREG32(&pOtgReg->ENDPTPRIME) & *pPrime)
    {
        ;
    }

    *pStat = INREG32(&pOtgReg->ENDPTSTATUS);
    if ( (bIN && ((stat.ETBR & 1) == 0)) || (!bIN && ((stat.ERBR & 1) == 0)) )
    {
        USB_ENDPTCOMPLETE_T edptcomp;
        DWORD *pEpc;
        RETAILMSG(DEBUG_LOG_USBCV, (L"Fail HandShake\r\n"));
        pEpc = (DWORD*)&edptcomp;
        *pEpc = INREG32(&pOtgReg->ENDPTCOMPLETE);
        if ( (bIN && ((edptcomp.ETCE & 1) == 0)) ||
             (!bIN && ((edptcomp.ERCE & 1) == 0)) )
        {
            DEBUGMSG(ZONE_ERROR, (_T("################ Failed to prime for ZLT %s\r\n"),
                                  bIN ? TEXT("IN") : TEXT("OUT")));
            RETAILMSG(DEBUG_LOG_USBCV, (L"Failed to prime for ZLT\r\n"));
        }
    }

    pPdd->qhbuffer->bPrimed[endp] = TRUE;
}

//-------------------------------------------------------------
//
//  Function: HandleUSBReset
//
//  This is to handle the USB Reset interrupt
//
//  Parameters:
//      pPdd - Pointer to USBFN_PDD
//
//  Return: NULL
//
//------------------------------------------------------------------
static void
HandleUSBReset(USBFN_PDD *pPdd)
{
    volatile CSP_USB_REG * pOtgReg=&(pPdd->pUSBDRegs->OTG);
    USB_PORTSC_T state;
    DWORD * tPortsc;

    LOCK();

    // clear all setup notification bits
    OUTREG32(&pOtgReg->ENDPTSETUPSTAT, INREG32(&pOtgReg->ENDPTSETUPSTAT));

    // clear all complete bits
    RETAILMSG(DEBUG_LOG_USBCV, (L"Reset Clear EPCOMPLETE\r\n"));
    OUTREG32(&pOtgReg->ENDPTCOMPLETE, INREG32(&pOtgReg->ENDPTCOMPLETE));

    // wait for all prime (if any in progress) to complete, then flush all
    while ( INREG32(&pOtgReg->ENDPTPRIME) )
    {
        ;
    }

    OUTREG32(&pOtgReg->ENDPTFLUSH, 0xFFFFFFFF);

    // and check that we're still in RESET
    tPortsc = (DWORD*)&state;
    *tPortsc = INREG32(&pOtgReg->PORTSC[USBD_PORT_NUM]);
    if ( state.PR == 0 )
    {
        DEBUGMSG(ZONE_WARNING,(_T("HandleUSBReset: Not still in reset condition. Possibly reset controller??\r\n")));
    }

    // every time a reset happen, we assume a disconnect happen
    // if (pPdd->devState==0) // Why no bit set in PORTSC???
    {
        // Let MDD process change
        USB_PORTSC_T state;
        DWORD * t1;
        int i;

        DEBUGMSG(ZONE_PDD, (L"Device Attach\r\n"));
        //
        // Get device state & change
        t1=(DWORD *)&state;
        *t1 = INREG32(&pOtgReg->PORTSC[USBD_PORT_NUM]);

        pPdd->devState=1;

        UNLOCK();
        RETAILMSG(DEBUG_LOG_USBCV, (L"ATTACH1\r\n"));
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
        LOCK();

        if (state.PSPD==0x2)
        {
            UNLOCK();
            RETAILMSG(DEBUG_LOG_USBCV, (L"HS1\r\n"));
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_HIGH_SPEED);
            DEBUGMSG(ZONE_PDD,(_T(" BS_HIGH_SPEED   %x\r\n"),BS_HIGH_SPEED) );

            LOCK();
        }
        else
        {
            UNLOCK();
            RETAILMSG(DEBUG_LOG_USBCV, (L"FS1\r\n"));
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_FULL_SPEED);
            DEBUGMSG(ZONE_PDD,(_T(" BS_FULL_SPEED   %x\r\n"),BS_FULL_SPEED) );
            LOCK();
        }

        DEBUGMSG(ZONE_PDD,(_T(" state.PSPD %x\r\n"),state.PSPD) );

        if (state.PSPD==2)
        {
            pPdd->highspeed=1;
            for (i=0;i<USBD_EP_COUNT;i++)
                pPdd->ep[i]=&pPdd->eph[i];
        }
        else
        {
            pPdd->highspeed=0;
            for (i=0;i<USBD_EP_COUNT;i++)
                pPdd->ep[i]=&pPdd->epf[i];
        }
    }


    UNLOCK();

    pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, (DWORD)UFN_RESET);

    // no need to free TDs since they are statically allocated.
}


//----------------------------------------------------------------
//
//  Function:  UfnPdd_PowerDown
//
//  This is power down function for USB PDD function driver
//
//  Parameters:
//      pPddContext - PDD Context
//
//  Return:
//      NULL
//
//----------------------------------------------------------------
VOID WINAPI UfnPdd_PowerDown(VOID *pPddContext)
{
    USBFN_PDD *pPdd = (USBFN_PDD *)(pPddContext);
    DEBUGMSG(ZONE_POWER, (L"UsbPdd_PowerDown\r\n"));
    if (pPdd->bInUSBFN)
    {
        USB_CTRL_T ctrl;
        USB_PORTSC_T portsc;
        USB_USBCMD_T usbcmd;
        DWORD *temp3;
        DWORD *temp2;
        DWORD *temp;

        if (pPdd->bUSBCoreClk == FALSE)
        {
            // We know it is suspend already ...why bother to
            // further processing anything.
            // We use bUSBCoreClk as checking since this is the last thing to
            // do b4 we go to suspend.
            DEBUGMSG(ZONE_PDD, (TEXT("UfnPdd_PowerDown no action required\r\n")));
            return;
        }

        // Stop USB Run/Stop CMD
        temp2 = (DWORD *)&usbcmd;
        *temp2 = INREG32(&pPdd->pUSBDRegs->OTG.USBCMD);
        usbcmd.RS = 0;
        OUTREG32(&pPdd->pUSBDRegs->OTG.USBCMD, *temp2);
        DEBUGMSG(ZONE_POWER, (TEXT("PowerDown USBCMD = 0x%x\r\n"), INREG32(&pPdd->pUSBDRegs->OTG.USBCMD)));

        // Clear USB Interrupt.
        pPdd->dwUSBIntrValue = INREG32(&pPdd->pUSBDRegs->OTG.USBINTR);
        OUTREG32(&pPdd->pUSBDRegs->OTG.USBINTR, 0x00);
        DEBUGMSG(ZONE_POWER, (TEXT("PowerDown USBINTR = 0x%x\r\n"),
                              INREG32(&pPdd->pUSBDRegs->OTG.USBINTR)));

        // Clear USBSTS
        OUTREG32(&pPdd->pUSBDRegs->OTG.USBSTS,
                 INREG32(&pPdd->pUSBDRegs->OTG.USBSTS));


        temp = (DWORD *)&portsc;
        *temp = INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]);
        portsc.WKOC = 1;
        portsc.WKDC = 1;
        portsc.WKCN = 1;
        OUTREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0], *temp);
        DEBUGMSG(ZONE_POWER, (TEXT("PowerDown PORTSC= 0x%x\r\n"), INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0])));

        // Switch ULPI to suspend mode
        //SetPHYPowerMgmt(pPdd->pUSBDRegs, TRUE);

        // Disable wakeup condition
        temp3 = (DWORD *)&ctrl;
        *temp3 = INREG32(&pPdd->pUSBDRegs->USB_CTRL);

        // We have to disable the wakeup, or it will fail in following
        // condition:
        // - Unplug the USB device cable and plug in a DOK
        // - Unplug the DOK and wake it up by pressing the power on button
        //ctrl.OWIE = 1;
        //ctrl.OUIE = 1;
        ctrl.OWIE = 0;
        ctrl.OUIE = 0;

        OUTREG32(&pPdd->pUSBDRegs->USB_CTRL, *temp3);
        DEBUGMSG(ZONE_POWER, (TEXT("PowerDown USBCTRL=0x%x\r\n"), INREG32(&pPdd->pUSBDRegs->USB_CTRL)));

        // Set PORTSC PHCD to 1
        temp = (DWORD *)&portsc;
        *temp = INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]);
        portsc.PHCD = 1;
        OUTREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0], *temp);
        DEBUGMSG(ZONE_POWER, (TEXT("PowerDown PORTSC= 0x%x\r\n"),
                              INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0])));

        // We need to wait for SUSPEND bit set
        //while((INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]) & 0x80) == 0x00)
        //  Sleep(0);
        if(!pPdd->bIsMX31TO2)
        {
            if (pPdd->bUSBPanicMode)
            {
                DEBUGMSG(ZONE_POWER, (TEXT("Panic Mode off\r\n")));
                pPdd->bUSBPanicMode = FALSE;
                DDKClockDisablePanicMode();
            }
        }

        // Stop USB Clock now
        if (pPdd->bUSBCoreClk)
        {
            DEBUGMSG(ZONE_POWER, (TEXT("USB Clock stop\r\n")));
            pPdd->bUSBCoreClk = FALSE;
            BSPUSBClockDisable(TRUE);
        }
    }
    DEBUGMSG(ZONE_POWER, (TEXT("PowerDown completed\r\n")));
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_PowerUp
//
//  This function is called during Power Up
//
//  Parameters:
//     pPddContext - Pointer to USBFN_PDD
//
//  Return:
//     NULL
//
//------------------------------------------------------------------------------
VOID WINAPI UfnPdd_PowerUp(VOID *pPddContext)
{
    USBFN_PDD *pPdd = (USBFN_PDD *)(pPddContext);

    DEBUGMSG(ZONE_POWER, (_T("UsbPdd_PowerUp with bInUSBFN %d\r\n"),
                          pPdd->bInUSBFN) );

    if (pPdd->bInUSBFN)
    {

        USB_CTRL_T ctrl;
        USB_USBCMD_T usbcmd;
        DWORD *temp3;
        DWORD *temp2;

        // Start USB Clock now
        if (pPdd->bUSBCoreClk == FALSE)
        {
            DEBUGMSG(ZONE_POWER, (TEXT("USB Clock start\r\n")));
            pPdd->bUSBCoreClk = TRUE;
            BSPUSBClockDisable(FALSE);
        }

        if(!pPdd->bIsMX31TO2)
        {
                if (pPdd->bUSBPanicMode == FALSE)
                {
                        DEBUGMSG(ZONE_POWER, (TEXT("Panic Mode On\r\n")));
                        pPdd->bUSBPanicMode = TRUE;
                        DDKClockEnablePanicMode();
                }
        }

        DEBUGMSG(ZONE_POWER, (TEXT("PowerUp:USBSTS(0x%x) PORTSC(0x%x)\r\n"),
                              INREG32(&pPdd->pUSBDRegs->OTG.USBSTS),
                              INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]) ));

        // Force resume first.
        if (INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]) & 0x800000)
        {
            // Clear the PHCD
            CLRREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0],
                     INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]) & 0x800000);

            // Force Resume bit
            OUTREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0],
                     INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0])|0x40);

            DEBUGMSG(ZONE_POWER, (TEXT("Force resume\r\n")));
        }

        //Disable wakeup condition
        temp3 = (DWORD *)&ctrl;
        *temp3 = INREG32(&pPdd->pUSBDRegs->USB_CTRL);

        DEBUGMSG(ZONE_POWER, (TEXT("PowerUp wakeup = 0x%x\r\n"), *temp3));
        if (ctrl.OWIR == 1)
        {
            *temp3 = 0;
            ctrl.OWIE = 1;
            CLRREG32(&pPdd->pUSBDRegs->USB_CTRL, *temp3);
        }

        // Switch ULPI to resume mode
        //SetPHYPowerMgmt(pPdd->pUSBDRegs, FALSE);

        // Enable back all interrupt
        OUTREG32(&pPdd->pUSBDRegs->OTG.USBINTR, pPdd->dwUSBIntrValue);
        DEBUGMSG(ZONE_POWER, (TEXT("Wakeup Interrupt Setting 0x%x\r\n"),
                 pPdd->dwUSBIntrValue));

        // Start USB Run/Stop CMD
        temp2 = (DWORD *)&usbcmd;
        *temp2 = INREG32(&pPdd->pUSBDRegs->OTG.USBCMD);
        usbcmd.RS = 1;
        OUTREG32(&pPdd->pUSBDRegs->OTG.USBCMD, *temp2);
        DEBUGMSG(ZONE_POWER, (TEXT("PowerUp USBCMD = 0x%x\r\n"),
                              INREG32(&pPdd->pUSBDRegs->OTG.USBCMD)));

        pPdd->bResume = TRUE;
        SetInterruptEvent(pPdd->sysIntr);
    }
}

//------------------------------------------------------------------------------
//
// Function: UpdateDevicePower
//
// UpdateDevice according to self power state and PM power state.
//
// Parameter:
//    pPdd - Pointer to USBFN_PDD
//
// Return:
//    Current Power state
//
//------------------------------------------------------------------------------
CEDEVICE_POWER_STATE UpdateDevicePower(USBFN_PDD *pPdd)
{
    CEDEVICE_POWER_STATE cpsNew;
    PREFAST_ASSERT(pPdd!=NULL);
    cpsNew= min (pPdd->m_CurPMPowerState, pPdd->m_CurSelfPowerState);

    DEBUGMSG(ZONE_FUNCTION, (_T("UpdateDevicePower Going from D%d to D%d\r\n"),
                             pPdd->m_CurActualPowerState , cpsNew));

    if ( (cpsNew < pPdd->m_CurActualPowerState) && pPdd->hParentBus)
    {
        BOOL bBusSucceed = SetDevicePowerState(pPdd->hParentBus, cpsNew, NULL);
        if (bBusSucceed &&
            (pPdd->m_CurActualPowerState==D3 || pPdd->m_CurActualPowerState==D4))
        {
            DEBUGMSG(ZONE_PDD, (TEXT("UpdateDevicePower to powerup\r\n")));

            UfnPdd_PowerUp((PVOID)pPdd);
        }
    }

    if ( (cpsNew > pPdd->m_CurActualPowerState ) && pPdd->hParentBus  )
    {
        BOOL bBusSucceed = SetDevicePowerState(pPdd->hParentBus, cpsNew, NULL);
        if (bBusSucceed && (cpsNew == D4 ||cpsNew == D3 ))
        {
            DEBUGMSG(ZONE_PDD, (TEXT("UpdateDevicePower to powerdown\r\n")));

            UfnPdd_PowerDown((PVOID)pPdd);
        }
    }

    pPdd->m_CurActualPowerState = cpsNew ;
    return (cpsNew) ;
}


//------------------------------------------------------------------------------
//
//  Function:  SetupInterrupt
//
//  This function handles setup packet interrupts.
//
//  Parameter:
//      pPdd - Pointer to USBFN_PDD
//
//  Return:
//      NULL
//
//------------------------------------------------------------------------------
static VOID SetupEvent(USBFN_PDD *pPdd)
{
    CSP_USB_REGS*pUSBDRegs = pPdd->pUSBDRegs;
    DWORD data[2];

    LOCK();
    if (pPdd->devState==0) // Why no bit set in PORTSC???
    {
        // Let MDD process change
        USB_PORTSC_T state;
        DWORD * t1;
        int i;

        RETAILMSG(DEBUG_LOG_USBCV, (L"Proc Attach at Setup\r\n"));
        DEBUGMSG(ZONE_PDD, (L"Device Attach\r\n"));

        // Get device state & change
        t1=(DWORD *)&state;
        *t1 = INREG32(&pUSBDRegs->OTG.PORTSC[USBD_PORT_NUM]);

        pPdd->devState=1;

        UNLOCK();
#ifdef SUSPEND_WITH_ATA
#define SLEEP_MACRO_1 3000
        Sleep(SLEEP_MACRO_1);
#endif
        RETAILMSG(DEBUG_LOG_USBCV, (L"ATTACH2\r\n"));
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
        LOCK();

        if (state.PSPD==0x2)
        {
            UNLOCK();
            RETAILMSG(DEBUG_LOG_USBCV, (L"HS2\r\n"));
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_HIGH_SPEED);
            LOCK();
        }
        else
        {
            UNLOCK();
            RETAILMSG(DEBUG_LOG_USBCV, (L"FS2\r\n"));
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_FULL_SPEED);
            LOCK();
        }

        if (state.PSPD==2)
        {
            pPdd->highspeed=1;
            for (i=0;i<USBD_EP_COUNT;i++)
                pPdd->ep[i]=&pPdd->eph[i];
        }
        else
        {
            pPdd->highspeed=0;
            for (i=0;i<USBD_EP_COUNT;i++)
                pPdd->ep[i]=&pPdd->epf[i];
        }
    }

    GetSetupPacket(pPdd, data);

    DEBUGMSG(ZONE_PDD,(_T("Setup: %x %x\r\n"),data[0],data[1]) );

    UNLOCK();

    if (!CheckAndHandleUnsupportedRequests(pPdd, (PUSB_DEVICE_REQUEST) &data))
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SETUP_PACKET, (DWORD)data);
}


//----------------------------------------------------------------
//
//  Function:  DumpUSBRegs
//
//  Dump all the registers in MX31 usb memory map
//
//  Parameter:
//      pUSBDRegs - pointer to usb memory map
//
//  Return
//     NULL
//
//----------------------------------------------------------------
void DumpUSBRegs(CSP_USB_REGS* pUSBDRegs)
{
    // offset 0x000 ~ 0x604
    DWORD offset;
    DWORD rangeS = 0x0, rangeE = 0x1f;

#ifdef SHIP_BUILD
    UNREFERENCED_PARAMETER(pUSBDRegs);
#endif


    RETAILMSG(1, (L"USBCMD %x\r\n", INREG32(&pUSBDRegs->OTG.USBCMD)));
    RETAILMSG(1, (L"USBSTS %x\r\n", INREG32(&pUSBDRegs->OTG.USBSTS)));
    RETAILMSG(1, (L"PORTSC %x\r\n", INREG32(&pUSBDRegs->OTG.PORTSC)));
    RETAILMSG(1, (L"USBINTR %x\r\n", INREG32(&pUSBDRegs->OTG.USBINTR)));
    RETAILMSG(1, (L"USBCTRL %x\r\n", INREG32(&pUSBDRegs->USB_CTRL)));

    RETAILMSG(1, (L"%4x ~ %4x\t", rangeS, rangeE));
    for (offset = 0; offset <= 0x604; offset += 4)
    {
        RETAILMSG(1, (L"%12x", INREG32((PUCHAR)(pUSBDRegs) + offset)));
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

//----------------------------------------------------------------
//
//  Function:  DumpDeviceState
//
//  Dump the connection status based on the PORTSC register
//
//  Parameter:
//     state - Pointer to PORTSC register
//
//  Return
//     NULL
//
//----------------------------------------------------------------
#ifdef DEBUG
void DumpDeviceState( USB_PORTSC_T * state)
{
    if (state->CCS)
        RETAILMSG(1, (L"\t\tCurrent Connect Status: Attached\r\n"));
    if (state->CSC)
        RETAILMSG(1, (L"\t\tConnect Status Change: Changed\r\n"));
    if (state->PE)
        RETAILMSG(1, (L"\t\tPort Enabled\r\n"));
    if (state->PEC)
        RETAILMSG(1, (L"\t\tPort Enable/Disable Change\r\n"));
    if (state->OCA)
        RETAILMSG(1, (L"\t\tOver-current Active\r\n"));
    if (state->OCC)
        RETAILMSG(1, (L"\t\tOver-current Change\r\n"));
    if (state->FPR)
        RETAILMSG(1, (L"\t\tForce Port Resume\r\n"));
    if (state->SUSP)
        RETAILMSG(1, (L"\t\tSuspend\r\n"));
    if (state->PR)
        RETAILMSG(1, (L"\t\tPort Reset\r\n"));
    if (state->HSP)
        RETAILMSG(1, (L"\t\tHigh-Speed Port \r\n"));

    RETAILMSG(1, (L"\t\tLine Status: %x", state->LS));
    switch (state->LS)
    {
    case 0:
        RETAILMSG(1, (L"\t\t\tSE0\r\n"));
        break;

    case 1:
        RETAILMSG(1, (L"\t\t\tJ-state\r\n"));
        break;

    case 2:
        RETAILMSG(1, (L"\t\t\tK-state\r\n"));
        break;

    case 3:
    default:
        RETAILMSG(1, (L"\t\t\tUndefined\r\n"));
        break;
    }

    if (state->PP)
        RETAILMSG(1, (L"\t\t??? Should be 0 for device\r\n"));

    if (state->PO)
        RETAILMSG(1, (L"\t\tPort Owner\r\n"));

    if (state->PIC)
    {
        RETAILMSG(1, (L"\t\tPort Indicator Control"));
        switch (state->PIC)
        {
        case 1:
            RETAILMSG(1, (L"\t\t\tAmber\r\n"));
            break;

        case 2:
            RETAILMSG(1, (L"\t\t\tGreen\r\n"));
            break;

        case 3:
        default:
            RETAILMSG(1, (L"\t\t\tUndefined\r\n"));
            break;
        }
    }
    if (state->PTC)
        RETAILMSG(1, (L"\t\tPort Test Control: %x\r\n", state->PTC));

    if (state->WKCN)
        RETAILMSG(1, (L"\t\tWake on Connect Enable (WKCNNT_E)\r\n"));

    if (state->WKDC)
        RETAILMSG(1, (L"\t\tWake on Disconnect Enable (WKDSCNNT_E) \r\n"));

    if (state->WKOC)
        RETAILMSG(1, (L"\t\tWake on Over-current Enable (WKOC_E) \r\n"));

    if (state->PHCD)
        RETAILMSG(1, (L"\t\tPHY Low Power Suspend - Clock Disable (PLPSCD) \r\n"));

    if (state->PFSC)
        RETAILMSG(1, (L"\t\tPort Force Full Speed Connect \r\n"));

    RETAILMSG(1, (L"\t\tPort Speed: %x->", state->PSPD));
    switch (state->PSPD)
    {
    case 0:
        RETAILMSG(1, (L"\t\t\tFull Speed\r\n"));
        break;

    case 1:
        RETAILMSG(1, (L"\t\t\tLow Speed\r\n"));
        break;

    case 2:
        RETAILMSG(1, (L"\t\t\tHigh Speed\r\n"));
        break;

    case 3:
    default:
        RETAILMSG(1, (L"\t\t\tUndefined\r\n"));
        break;
    }

    RETAILMSG(1, (L"\t\tParallel Transceiver Width:%x->", state->PTW));

    if (state->PTW)
        RETAILMSG(1, (L"\t\t\t16 bits\r\n"));
    else
        RETAILMSG(1, (L"\t\t\t8 bits\r\n"));

    if (state->STS)
        RETAILMSG(1, (L"\t\tSerial Transceiver Select \r\n"));

    RETAILMSG(1, (L"\t\tParallel Transceiver Select:%x->", state->PTS));

    switch (state->PTS)
    {
    case 0:
        RETAILMSG(1, (L"\t\t\tUTMI/UTMI+\r\n"));
        break;

    case 1:
        RETAILMSG(1, (L"\t\t\tPhilips Classic\r\n"));
        break;

    case 2:
        RETAILMSG(1, (L"\t\t\tULPI\r\n"));
        break;

    case 3:
        RETAILMSG(1, (L"\t\t\tSerial/1.1 PHY (FS Only)\r\n"));
        break;
    default:
        RETAILMSG(1, (L"\t\t\tUndefined\r\n"));
        break;

    }
}


//------------------------------------------------------------------------------
//
//  Function:  DumpInterruptSource
//
//  This function dump the value on the interrupt register USBSTS
//
//
//  Parameters:
//
//      source - pointer to USBSTS
//
//  Return:
//
//      NULL
//
//-------------------------------------------------------------------------------
void DumpInterruptSource(USB_USBSTS_T * source)
{
    if (source->UI)
        RETAILMSG(1, (L"\tUSB Interrupt (USBINT)\r\n"));
    if (source->UEI)
        RETAILMSG(1, (L"\tUSB Error Interrupt (USBERRINT)\r\n"));
    if (source->PCI)
        RETAILMSG(1, (L"\tPort Change Detect\r\n"));
    if (source->FRI)
        RETAILMSG(1, (L"\tFrame List Rollover\r\n"));
    if (source->SEI)
        RETAILMSG(1, (L"\tSystem Error\r\n"));
    if (source->AAI)
        RETAILMSG(1, (L"\tInterrupt on Async Advance\r\n"));
    if (source->URI)
        RETAILMSG(1, (L"\tUSB Reset Received \r\n"));
    if (source->SRI)
        RETAILMSG(1, (L"\tSOF Received \r\n"));
    if (source->SLI)
        RETAILMSG(1, (L"\tDCSuspend \r\n"));
    if (source->ULPII)
        RETAILMSG(1, (L"\tULPI Interrupt\r\n"));
    if (source->HCH)
        RETAILMSG(1, (L"\tHCHaIted \r\n"));
    if (source->RCL)
        RETAILMSG(1, (L"\tReclamation,Only used by the host controller. \r\n"));
    if (source->PS)
        RETAILMSG(1, (L"\tPeriodic Schedule Status. \r\n"));
    if (source->AS)
        RETAILMSG(1, (L"\tAsynchronous Schedule Status. \r\n"));
}

#endif
//------------------------------------------------------------------------------
//
//  Function:  DevStatEvent
//
//  This function handles device state change interrupts.
//
//  Called with context LOCKed
//
//  Parameters:
//
//      pPdd - Pointer to USBFN_PDD structure
//
//  Return:
//
//      NULL
//
//-------------------------------------------------------------------------------
static VOID DevStatEvent (USBFN_PDD *pPdd)
{
    CSP_USB_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD *t1;
    USB_PORTSC_T state;

    // Get device state & change
    t1=(DWORD *)&state;
    *t1 = INREG32(&pUSBDRegs->OTG.PORTSC[USBD_PORT_NUM]);

    //DumpDeviceState( & state);
    //Reset

    //Deattach
    if (pPdd->devState&&state.SUSP)
    {
        // TODO: Call bus driver (OTG?) to move HW to deep sleep
        // Let MDD process change
        RETAILMSG(DEBUG_LOG_USBCV, (L"SUSPEND\r\n"));
        DEBUGMSG(ZONE_PDD, (_T("Device Detach\r\n")));

#ifdef USBCV_FIX
        RETAILMSG(DEBUG_LOG_USBCV, (L"GIVE SUSPEND\r\n"));
#else
        // Don't process other changes (we are disconnected)
        pPdd->devState=0;

        UNLOCK();
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
        LOCK();

        // SetSelfPowerState to D4
        pPdd->m_CurSelfPowerState = D4; // Do we need set to D3 as wake up source?
        UpdateDevicePower(pPdd);

        OUTREG32(&pPdd->pUSBDRegs->OTG.T_154H.USBADR, 0);
#endif
    }
}

//------------------------------------------------------------------------------
//
//  Function:  CopyFromUncachedBuffer
//
//  This function copies the data from the uncached memory back to the user buffers
//
//
//  Parameters:
//
//      pPdd - Pointer to USBFN_PDD structure
//
//      EpNum - endpoint for which copy needs to be done
//
//      len - length of data to be copied
//
//  Return:
//
//      NULL
//
//-------------------------------------------------------------------------------
void CopyFromUncachedBuffer(USBFN_PDD *pPdd, int EpNum, DWORD len )
{

    DWORD dwOffset = ((DWORD)pPdd->ep[EpNum]->pMappedBufPtr) & (UserKInfo[KINX_PAGESIZE] - 1);
    DWORD TdLen, BPCount, EndDataSize, BuffLen = len;

    //RETAILMSG(1,(_T("inside CopyFromUncachedBuffer for EP = %d, dwOffset = %d\r\n"),EpNum,dwOffset));

    // Get the length transfered in the first page, from the offset
    if (len > (MAX_SIZE_PER_BP - dwOffset))
    {
        TdLen = (MAX_SIZE_PER_BP - dwOffset);
        len -= TdLen;
    }
    else
    {
        TdLen = len;
        len = 0;   // first bp contains all the data
    }

    //If first page had all the data, then need to check if offset or the length is aligned
    if ((dwOffset % 32) != 0 || ((BuffLen < (MAX_SIZE_PER_BP - dwOffset)) && ((BuffLen % 32) != 0)))
    {
        RETAILMSG(1,(_T("Copying for bp0: offset: %x, len %x\r\n"), dwOffset, BuffLen));
        memcpy(pPdd->ep[EpNum]->pMappedBufPtr, ((PUCHAR)Buffer_Td1 + dwOffset), TdLen );
    }

    // If the data > 16k then need to check for the Endsize that might have been unaligned,
    // need this only if the Endsize comes in the same page where a 16k size ends.
    if(BuffLen > MAX_SIZE_PER_TD)
    {
        EndDataSize = BuffLen % MAX_SIZE_PER_TD;
        if((EndDataSize + dwOffset) <= MAX_SIZE_PER_BP)
        {
            if ((EndDataSize % 32) != 0)
            {
                memcpy((pPdd->ep[EpNum]->pMappedBufPtr + ( (pPdd->ep[EpNum]->dwNumPages - 1) * MAX_SIZE_PER_BP)), Buffer_Td2, EndDataSize );
            }
            // All the data has been checked for alignement , so no need to go in the flow.
            len = 0;
        }
    }

    // Come here if the Data is not over in the first page itself, or did not go in to the above condition

    BPCount = 1;  // First page has been taken care of , so start from second page

    while (len > 0)
    {
        // Flow through the user buffer till the end and then check for the alignment of the length
        if ( len > MAX_SIZE_PER_BP )
        {
            len -= MAX_SIZE_PER_BP;
            BPCount++;
        }
        else
        {
            if ((len % 32) != 0)
            {
                    memcpy((pPdd->ep[EpNum]->pMappedBufPtr + ((BPCount * MAX_SIZE_PER_BP) - dwOffset)), Buffer_Td2, len );
            }
            len = 0;
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function:  CheckEndpoint
//
//  This function handles the interrupt due to  endpoint complete
//
//
//  Parameters:
//
//      pPdd - Pointer to USBFN_PDD structure
//
//      i - endpoint to be examined
//
//  Return:
//      NULL
//
//------------------------------------------------------------------------------
static void CheckEndpoint(USBFN_PDD *pPdd, int i )
{
    CSP_USB_REGS* pUSBDRegs = pPdd->pUSBDRegs;
    STransfer * pTransfer;
    USB_ENDPTCOMPLETE_T edptcomp;
    DWORD *pEpc;

    pEpc = (DWORD*)&edptcomp;

    LOCK();
    // OUT endpoint
    *pEpc = INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE);
    RETAILMSG(FALSE, (L"check EP %x\r\n", *pEpc));

    if (edptcomp.ERCE&(1<<i))
    {
#ifdef USBCV_FIX
        ClearUsbstsUI(pUSBDRegs);
#endif
        RETAILMSG(DEBUG_LOG_USBCV, (L"R on %d\r\n", i));
        pTransfer=pPdd->ep[i]->pTransfer;
        if (i)
        {
            PUSBD_dTD_R_T pTd;
            PUSBD_dQH_T pQh;

            pQh=&(pPdd->qhbuffer->qh[i*2]);
            pTd=TDVirtual(pQh->curr_dTD);
            if (pPdd->qhbuffer->qh[i*2].dtd.status)
            {
                DEBUGMSG(ZONE_PDD, (L"Status is not 0 on recv %x =%x",
                                    i, pPdd->qhbuffer->qh[i*2].dtd.status));
            }

            if (pTransfer&&pPdd->qhbuffer->qh[i*2].dtd.status!=0x80)
            {
                // calculate how much we were trying to tranfer in this TD
                DWORD len=pTransfer->cbBuffer-pTransfer->cbTransferred;

                if (len>MAX_SIZE_PER_TD)
                    len=MAX_SIZE_PER_TD;

                len-=pPdd->qhbuffer->qh[i*2].dtd.tb;  // reduce by any bytes outstanding

                // len is how much was actually transferred

                pTransfer->cbTransferred+=len;

                DEBUGMSG(ZONE_PDD, (TEXT("ENDPTCOMPLETE %x\r\n"),(1<<i)));
                RETAILMSG(DEBUG_LOG_USBCV, (L"clear EPC 1\r\n"));
                OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), (1<<i));

                if (pPdd->qhbuffer->qh[i*2].dtd.status && pPdd->qhbuffer->qh[i*2].dtd.status!=0x80)
                {
                    RETAILMSG(FALSE, (L"Buffer Error\r\n"));
                    DEBUGMSG(ZONE_ERROR,(_T("CheckEndpoint(ep=%d): BUFFER_ERROR\r\n"), i) );

                    pTransfer->dwUsbError=UFN_CLIENT_BUFFER_ERROR;
                }
                else if ( (len == MAX_SIZE_PER_TD) &&
                          (pTransfer->cbTransferred < pTransfer->cbBuffer) )
                {
                    /* RETAILMSG(1, (_T("More data need to send (0x%x) ep(%d)\r\n"),
                    pTransfer->cbBuffer-pTransfer->cbTransferred, i)); */

                    RETAILMSG(DEBUG_LOG_USBCV, (L"More..\r\n"));
                    pTransfer->dwUsbError=UFN_MORE_DATA;

                    UNLOCK();
                    UfnPdd_IssueTransfer(pPdd, i,pTransfer);
                    LOCK();
                }
                else
                {
                    RETAILMSG(DEBUG_LOG_USBCV, (L"finish %d bytes\r\n", pTransfer->cbTransferred));
                    pTransfer->dwUsbError=UFN_NO_ERROR;
                }

                pPdd->qhbuffer->bPrimed[i] = FALSE;

                if (pTransfer->dwUsbError != UFN_MORE_DATA )
                {
                    CacheSync(CACHE_SYNC_DISCARD);

                    //CacheRangeFlush(pTransfer->pvBuffer, pTransfer->cbBuffer, CACHE_SYNC_DISCARD);
                    // For copying from Uncached memory to user buffers
                    if ( pTransfer->cbBuffer > 0 )
                    {
                        CopyFromUncachedBuffer(pPdd,i,pTransfer->cbBuffer);
                    }
                    if ( pPdd->ep[i]->bPagesLocked )
                    {
                        DEBUGCHK( pPdd->ep[i]->pMappedBufPtr != NULL );
                        UnlockPages(pPdd->ep[i]->pMappedBufPtr, pTransfer->cbBuffer);
                        pPdd->ep[i]->bPagesLocked = 0;
                    }

                    pPdd->ep[i]->pTransfer=NULL;

                    UNLOCK();
                    pPdd->pfnNotify(pPdd->pMddContext,
                                    UFN_MSG_TRANSFER_COMPLETE,
                                    (DWORD)pTransfer);

                    DEBUGMSG(ZONE_PDD, (TEXT("OUT %s Completed for endpoint %d\r\n"),
                                pTransfer->dwUsbError == UFN_NO_ERROR ? TEXT("Normal") : TEXT("Cancel"),
                                i));
                    LOCK();
                }
            }
            else //(pTransfer&&pPdd->qhbuffer->qh[i*2].dtd.status!=0x80)
            {
                RETAILMSG(DEBUG_LOG_USBCV, (L"why we are here 1\r\n"));
                DEBUGMSG(ZONE_PDD, (TEXT("############# ENDPTCOMPLETE %x NULL ")
                                    TEXT("pTransfer or bad status (0x%x)!\r\n"),
                                    (1<<i),
                                    pPdd->qhbuffer->qh[i*2].dtd.status));

                OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), (1<<i));
            }
        }
        else // Endpoint 0 OUT Transaction if (i)
        {
            DWORD len;

            // OUT transcation of endpoint 0
            //   pTransfer=pPdd->ep[USBD_EP_COUNT-1]->pTransfer;

            pTransfer=pPdd->ep[0]->pTransfer;
            if (pTransfer)
            {
                len=pTransfer->cbBuffer-pPdd->qhbuffer->td[0].tb;
                if (len)
                {
                    pTransfer->cbTransferred+=len;

                    pPdd->qhbuffer->bPrimed[0] = FALSE;
                    if (pTransfer->cbTransferred < pTransfer->cbBuffer)
                    {
                         DEBUGMSG(ZONE_PDD,(L"##### more data to receive on EP0 (status=0x%x)",
                         pPdd->qhbuffer->td[0].status));

                        RETAILMSG(DEBUG_LOG_USBCV, (L"more data 2\r\n"));
                        OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), 1);

                        UNLOCK();
                        UfnPdd_IssueTransfer(pPdd, 0,pTransfer);
                        LOCK();

                        // EP0 is treated just like any other EP at this
                        // point (no setup phase ready).
                    }
                    else
                    {
                        RETAILMSG(DEBUG_LOG_USBCV, (L"EP0 R complete\r\n"));
                        pTransfer->dwUsbError=UFN_NO_ERROR;

                        OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), 1);

                        // For copying from Uncached memory buffers to user buffers
                        if ( pTransfer->cbBuffer > 0 )
                        {
                            CopyFromUncachedBuffer(pPdd,0,pTransfer->cbBuffer);
                        }
                        CacheSync(CACHE_SYNC_DISCARD);

                        if ( pPdd->ep[0]->bPagesLocked && pTransfer->pvBuffer )
                        {
                            DEBUGCHK( pPdd->ep[0]->pMappedBufPtr != NULL );
                            UnlockPages(pPdd->ep[0]->pMappedBufPtr,
                                        pTransfer->cbBuffer);
                            pPdd->ep[0]->bPagesLocked = 0;
                        }

                        pPdd->ep[0]->pTransfer=NULL;

                        DEBUGMSG(ZONE_PDD, (TEXT("CheckEndpoint:EP0 OUT %d bytes notify (%s)\r\n"),
                                    pTransfer->cbTransferred,
                                    (pTransfer->dwUsbError == UFN_NO_ERROR) ? TEXT("Normal") : TEXT("Cancel")
                                    ) );

                        UNLOCK();
                        pPdd->pfnNotify(pPdd->pMddContext,
                                        UFN_MSG_TRANSFER_COMPLETE,
                                        (DWORD)pTransfer);
                        LOCK();
                    }
                }
                else
                {
                    RETAILMSG(DEBUG_LOG_USBCV, (L"EP0 R zero length\r\n"));
                    OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), 1);
                }

                DEBUGMSG(ZONE_PDD, (_T("CheckEndpoint:EP0 OUT %d bytes completed\r\n"), pTransfer->cbTransferred) );
            }
            else // if (pTransfer)
            {
                RETAILMSG(DEBUG_LOG_USBCV, (L"why here 2\r\n"));
                //The NAK interrupt is enabled to workaround the issue where the Device controller does not ACK
                //the repeated status stages but NAKs them.
                if(pPdd->StatusOutPrimed)
                {
                        //Enable Nak enable bit
                        OUTREG32(&pUSBDRegs->OTG.ENDPTNAKEN, 0x1);
                        pPdd->StatusOutPrimed = FALSE;
                }
                OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), 1);
                 DEBUGMSG(ZONE_PDD, (_T("CheckEndpoint:EP0 OUT NULL transfer complete\r\n")) );
            }
        } // if (pTransfer&&pPdd->qhbuffer->qh[i*2].dtd.status!=0x80)
    } //  if (edptcomp.ERCE&(1<<i))

    // IN packet transcation
    if (edptcomp.ETCE&(1<<i))
    {
        DWORD mapped_epNum = i;

#ifdef USBCV_FIX
        ClearUsbstsUI(pUSBDRegs);
#endif

        RETAILMSG(DEBUG_LOG_USBCV, (L"T on %d\r\n", i));
        if (i == 0)
        {
            /*
             * If this is the completion of a SetAddress (the status handshake
             * part is an "IN") then we should set the address now.  We only
             * set the address after the command. Address is & 0x80 as a flag
             * to tell us to set the address.
             */
            RETAILMSG(DEBUG_LOG_USBCV, (L"change dev addr to %x\r\n", pPdd->addr));
            if (pPdd->addr & 0x80)
            {
                DWORD addr;
                pPdd->addr&=0x7f;
                addr=pPdd->addr;
                addr<<=25;

                OUTREG32(&pPdd->pUSBDRegs->OTG.T_154H.USBADR, addr);

                DEBUGMSG(ZONE_PDD, (L"CheckEndpoint:Set Address: %x(%x), USBADR(%x)=%x \r\n",
                addr, pPdd->addr, offset(CSP_USB_REG, T_154H.USBADR) ,
                INREG32(&pPdd->pUSBDRegs->OTG.T_154H.USBADR)));
            }

            mapped_epNum = USBD_EP_COUNT-1;
        }

        pTransfer=pPdd->ep[mapped_epNum]->pTransfer;

        if (pPdd->qhbuffer->qh[i*2+1].dtd.status)
        {
            DEBUGMSG(ZONE_PDD, (L"Status is not 0 on sent %x =%x\r\n",
                                i,pPdd->qhbuffer->qh[i*2+1].dtd.status));
        }

        if (pTransfer && pPdd->qhbuffer->qh[i*2+1].dtd.status!=0x80)
        {
            // calculate how much we were trying to transfer
            int len=pTransfer->cbBuffer-pTransfer->cbTransferred;

            if (len>MAX_SIZE_PER_TD)
            {
                len=MAX_SIZE_PER_TD;
            }

            len -= pPdd->qhbuffer->qh[i*2+1].dtd.tb; // reduce by what's still to go in the TD

            pTransfer->cbTransferred += len;

            pPdd->qhbuffer->bPrimed[mapped_epNum] = FALSE;
            if (pPdd->qhbuffer->qh[i*2+1].dtd.tb)
            {
                RETAILMSG(DEBUG_LOG_USBCV, (L"why here 3\r\n"));
                DEBUGMSG(ZONE_PDD, (L"Send error?? len=%x, status=%x, tb=%x\r\n",
                                    pTransfer->cbBuffer,
                                    pPdd->qhbuffer->qh[i*2+1].dtd.status,
                                    pPdd->qhbuffer->qh[i*2+1].dtd.tb));

                pTransfer->cbTransferred=pTransfer->cbBuffer;
            }

            if (pTransfer->cbTransferred<pTransfer->cbBuffer)
            {
                RETAILMSG(DEBUG_LOG_USBCV, (L"more data 3\r\n"));
                pTransfer->dwUsbError=UFN_MORE_DATA;
                OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), (0x10000UL<<i));

                UNLOCK();
                UfnPdd_IssueTransfer(pPdd, i, pTransfer);
                LOCK();
            }
            else
            {
                RETAILMSG(DEBUG_LOG_USBCV, (L"complete %d bytes on T\r\n", pTransfer->cbTransferred));
                pTransfer->dwUsbError=UFN_NO_ERROR;

                OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), (0x10000UL<<i));

                if ( pPdd->ep[mapped_epNum]->bPagesLocked )
                {
                    DEBUGCHK( pPdd->ep[mapped_epNum]->pMappedBufPtr != NULL );
                    UnlockPages(pPdd->ep[mapped_epNum]->pMappedBufPtr,
                                pTransfer->cbBuffer);
                    pPdd->ep[mapped_epNum]->bPagesLocked = 0;
                }

                pPdd->ep[mapped_epNum]->pTransfer=NULL;
                UNLOCK();
                pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE,
                                (DWORD)pTransfer);
                LOCK();

                DEBUGMSG(ZONE_PDD, (TEXT("IN %s Completed at ep %d\r\n"),
                            pTransfer->dwUsbError == UFN_NO_ERROR ? TEXT("Normal") : TEXT("Cancel"),
                            i));
            }
        }
        else // (pTransfer&&pPdd->qhbuffer->qh[i*2+1].dtd.status!=0x80)
        {
            RETAILMSG(DEBUG_LOG_USBCV, (L"Why here 5?\r\n"));
            OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), (0x10000UL<<i));
        }
    } // else if (edptcomp.ETCE&(1<<i))

    UNLOCK();
}

/*
 *      Function : USBControllerRun
 *
 *      Description: This sets USB controller to either run or halted
 *
 *      Input :
 *            BOOL   bRunMode  -- if TRUE, then set running, else set stopped
 *
 *      Output:
 *            void
 *
 *      Return:
 *            ERROR_SUCCESS - okay, mode set
 *            ERROR_GEN_FAILURE -- couldn't set the mode for some reason
 *
 *
 *      Function waits until controller is stopped or started.
 */

static DWORD USBControllerRun( CSP_USB_REGS *pRegs, BOOL bRunMode )
{
    USB_USBCMD_T cmd;
    USB_USBMODE_T mode;
    DWORD *pTmpCmd = (DWORD*)&cmd;
    DWORD *pTmpMode = (DWORD*)&mode;

    *pTmpCmd = INREG32(&pRegs->OTG.USBCMD);
    if ( bRunMode )
    {
        DEBUGMSG(ZONE_PDD, (TEXT("Start USB controller (RS=%d -> RS=1)\r\n"),
                            cmd.RS));
        cmd.RS = 1;
    }
    else
    {
        DEBUGMSG(ZONE_PDD, (TEXT("Stop USB controller (RS=%d -> RS=0)\r\n"),
                            cmd.RS));
        cmd.RS = 0;
    }

    OUTREG32(&pRegs->OTG.USBCMD,*pTmpCmd);

    *pTmpMode = INREG32(&pRegs->OTG.USBMODE);
    if ( mode.CM == 0x3 )
    {
        // In host mode, make sure HCH (halted) already, and that mode
        // does change to halted (or not).
        USB_USBSTS_T stat;
        DWORD *pTmpStat = (DWORD*)&stat;
        int iAttempts;

        if ( bRunMode )
        {
            *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
            if ( stat.HCH == 0)
            {
                DEBUGMSG(ZONE_PDD,(TEXT("USBControllerRun(1): ERROR ")
                                   TEXT("############ HCH=0 (stat=0x%x)\r\n"),
                                   *pTmpStat));
                return ERROR_GEN_FAILURE;
            }
        }

        // wait for mode to change
        iAttempts = 0;
        do
        {
            *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
            if ( (!bRunMode && stat.HCH) || (bRunMode && (stat.HCH == 0)) )
            {
                return ERROR_SUCCESS;
            }

            Sleep(1);
        } while ( iAttempts++ < 50 );

        DEBUGMSG(ZONE_ERROR, (TEXT("USBControllerRun(%d): failed to set/clear ")
                              TEXT("RS\r\n"),bRunMode));
        return ERROR_GEN_FAILURE;
    }

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  InterruptThread
//
//  This is interrupt thread. It controls responsed to hardware interrupt. To
//  reduce code length it calls interrupt specific functions.
//
//  Parameter:
//
//      pPddContext - Pointer to USBFN_PDD
//
//  Return:
//
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------
static DWORD WINAPI InterruptThread(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS* pUSBDRegs = pPdd->pUSBDRegs;
    USB_USBSTS_T source;
    DWORD *temp;
#ifdef USBCV_FIX
    USB_OTGSC_T otgsc;
    DWORD* tOtgsc;
#endif
    int i;
    ULONG WaitReturn;
    TCHAR szUSBFunctionObjectName[30];
    TCHAR szUSBTransferObjectName[30];
    TCHAR szUSBXcvrObjectName[30];
    DWORD timeout = IDLE_TIMEOUT; // 3 sec
    DWORD StringSize;

    DEBUGMSG(ZONE_INTERRUPTS, (TEXT("IsOTGSupport? %d\r\n"), pPdd->IsOTGSupport));
    if (pPdd->IsOTGSupport)
    {
        StringSize = sizeof(szUSBFunctionObjectName) / sizeof(TCHAR);
        StringCchCopy(szUSBFunctionObjectName,StringSize,USBFunctionObjectName);
        //lstrcpy(szUSBFunctionObjectName, USBFunctionObjectName);
        StringCchCat(szUSBFunctionObjectName, StringSize, pPdd->szOTGGroup);
        //lstrcat(szUSBFunctionObjectName, pPdd->szOTGGroup);
        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFN: CreateEvent:%s\r\n"), szUSBFunctionObjectName));
        pPdd->hFunctionEvent = CreateEvent(NULL, FALSE, FALSE, szUSBFunctionObjectName);
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Opened an existing Func Event\r\n")));
        }
        else
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Created a new Func Event\r\n")));
        }

        if (pPdd->hFunctionEvent == NULL)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Create Event Failed for func!\r\n")));
        }

        StringSize = sizeof(szUSBXcvrObjectName) / sizeof(TCHAR);
        StringCchCopy(szUSBXcvrObjectName,StringSize,USBXcvrObjectName);
        //lstrcpy(szUSBXcvrObjectName, USBXcvrObjectName);
        StringCchCat(szUSBXcvrObjectName, StringSize, pPdd->szOTGGroup);
        //lstrcat(szUSBXcvrObjectName, pPdd->szOTGGroup);
        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFN: CreateEvent:%s\r\n"), szUSBXcvrObjectName));

        pPdd->hXcvrEvent = CreateEvent(NULL, FALSE, FALSE, szUSBXcvrObjectName);

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Opened an existing XCVR Event\r\n")));
        }
        else
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Created a new XCVR Event\r\n")));
        }

        if (pPdd->hXcvrEvent == NULL)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Create Event Failed for xcvr!\r\n")));
        }

        StringSize = sizeof(szUSBTransferObjectName) / sizeof(TCHAR);
        StringCchCopy(szUSBTransferObjectName,StringSize,USBTransferObjectName);
        StringCchCat(szUSBTransferObjectName, StringSize, pPdd->szOTGGroup);
        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFN: CreateEvent:%s\r\n"), szUSBTransferObjectName));

        pPdd->hTransferEvent = CreateEvent(NULL, FALSE, FALSE, szUSBTransferObjectName);
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Opened an existing Transfer Event\r\n")));
        }
        else
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Created a new Transfer Event\r\n")));
        }

        if (pPdd->hTransferEvent == NULL)
        {
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Create Event Failed for Transfer!\r\n")));
        }
XCVR_SIG:
        pPdd->bInUSBFN = FALSE;
        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UFN: Waiting for signal from XCVR!!!\r\n")));
        RETAILMSG(FALSE, (L"UFN block here...\r\n"));
        WaitReturn = WaitForSingleObject(pPdd->hFunctionEvent, INFINITE);
        RETAILMSG(FALSE, (L"UFN in control...\r\n"));
        // Exit thread when we are asked so...
        if (pPdd->exitIntrThread)
        {
            DEBUGMSG(ZONE_PDD, (L"InterruptThread: Exit Thread...\r\n"));
            return ERROR_SUCCESS;
        }
        pPdd->bInUSBFN = TRUE;

        //RETAILMSG(1, (TEXT("UFN: Function Driver in charge now!!!\r\n")));
        if (!InterruptInitialize(pPdd->sysIntr, pPdd->hIntrEvent, NULL, 0))
        {
            DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Interrupt initialization failed\r\n"));
            return FALSE;
        }

        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("UfnPdd_Start called\r\n")));

        // USB LOGO TEST does not allow a software detach here
        // So we can't resolve the XVC resume problem by adding a detach here
        // Resolution must be made on XVC part for the suspend resume problem
        UfnPdd_Start(pPdd);

        // Move this part to here so that it would only handle in case
        // of OTG support
        {
            USB_OTGSC_T otgsc_temp;
            DWORD       *t = (DWORD *)&otgsc_temp;

            *t = INREG32(&pUSBDRegs->OTG.OTGSC);
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("OTGSC IDIE(0x%x), IDIS(0x%x), (0x%x)\r\n"), otgsc_temp.IDIE, otgsc_temp.IDIS, *t));
            otgsc_temp.IDIE = 1;
            OUTREG32(&pUSBDRegs->OTG.OTGSC, *t);
        }

        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFNPDD SysIntr:0x%x, Irq:0x%x\r\n"), pPdd->sysIntr, pPdd->irq));

        // Handling of device attach
        temp=(DWORD *)&source;
        *temp = INREG32(&pUSBDRegs->OTG.USBSTS);
        // Clear source bit
        {
            do
            {
                if (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT) & 1)
                {
                    RETAILMSG(DEBUG_LOG_USBCV, (L"setup here?\r\n"));
                    SetupEvent(pPdd);
                }

                if (INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE))
                {
                    RETAILMSG(DEBUG_LOG_USBCV, (L"check ep here?\r\n"));
                    for (i=0;i<USBD_EP_COUNT;i++)
                    {
                        CheckEndpoint(pPdd, i );
                    }
                }
            } while (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT) ||
                     INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE));
        }
    }
#if 0
    /*This delay has moved to just before setup notification for suspend - resume operation*/
    {
        #define SLEEP_MACRO 2000
        RETAILMSG(1, (L"Sleep %d ms for ActiveSync Coldboot\r\n", SLEEP_MACRO));
        Sleep(2000);
    }
#endif

    while (!pPdd->exitIntrThread)
    {
        DWORD dwErr = 0;

        if (pPdd->IsOTGSupport)
        {
            timeout = INFINITE;
        }

        // Wait for interrupt from USB controller
        dwErr = WaitForSingleObject(pPdd->hIntrEvent, timeout);
        if (dwErr == WAIT_TIMEOUT)
        {
            // Now I am ready to put the transceiver into suspend mode.
            // But be aware there is a device attach when boot up.
            USB_CTRL_T ctrl;
            DWORD *temp3;
            USB_PORTSC_T portsc;
            DWORD *portsc_temp = (DWORD *)&portsc;

            *portsc_temp = INREG32(&(pUSBDRegs)->OTG.PORTSC);
            if ((portsc.SUSP == 0) && (pPdd->devState == 1))
            {
                // there is a device attached, don't do anything
                RETAILMSG(FALSE, (L"TimeOut_1\r\n"));
                timeout = INFINITE;
                continue;
            }

            // If there is a device attached at this time
            // Handling of device attach
            // Even though it is pure client, still, we have no other way
            // to isolate the host from the device, we need to use ID pin
            // to detect.
            if (portsc.SUSP == 0)
            {
                USB_OTGSC_T otgsc;
                DWORD *temp4 = (DWORD *)&otgsc;

                RETAILMSG(FALSE, (L"TimeOut_2\r\n"));
                *temp4 = INREG32(&(pUSBDRegs)->OTG.OTGSC);
                timeout = IDLE_TIMEOUT;
                if (otgsc.ID != 0)
                {
                    DEBUGMSG(ZONE_INTERRUPTS, (TEXT("Port is not suspend and no A-device ")
                                               TEXT("is detected\r\n")));

                    LOCK();
                    USBControllerRun(pUSBDRegs, FALSE);
                    HardwareInitialize(pUSBDRegs);
                    UfnPdd_Start(pPdd);
                    USBControllerRun(pUSBDRegs, TRUE);
                    UNLOCK();

                    goto PROCESS_INTERRUPT;
                }
            }

            // Now the device is not attached
            // Check and make sure all wakeup interrupt are enabled
            temp3 = (DWORD *)&ctrl;
            *temp3 = INREG32(&pUSBDRegs->USB_CTRL);
            ctrl.OWIE = 1;
            ctrl.OUIE = 1;
            OUTREG32(&pUSBDRegs->USB_CTRL, *temp3);

            // stop the controller before accessing the ULPI
            LOCK();
            USBControllerRun(pUSBDRegs, FALSE);

            // May need to set ULPI here
            // It is not waking up now ...must be something wrong
            *portsc_temp = INREG32(&(pUSBDRegs)->OTG.PORTSC);
            if (portsc.PHCD == 0)
            {
                RETAILMSG(FALSE, (L"TimeOut_3\r\n"));
                SetULPIToClientMode(pUSBDRegs);
            }

            portsc.PHCD = 1;
            OUTREG32(&(pUSBDRegs)->OTG.PORTSC, *portsc_temp);

            //DEBUGMSG(1, (TEXT("Lower core voltage now with portsc(0x%x) usbctrl(0x%x)\r\n"), INREG32(&(pUSBDRegs)->OTG.PORTSC), INREG32(&pUSBDRegs->USB_CTRL)));
            if(!pPdd->bIsMX31TO2)
            {
                BOOL bTemp;

                if (pPdd->bUSBPanicMode == TRUE)
                {
                    bTemp = DDKClockDisablePanicMode();
                    //RETAILMSG(1, (TEXT("DDKClockDisablePanicMode = 0x%x\r\n"), bTemp));
                    pPdd->bUSBPanicMode = FALSE;
                }
            }

            if (pPdd->bUSBCoreClk == TRUE)
            {
                pPdd->bUSBCoreClk = FALSE;
                BSPUSBClockDisable(TRUE);
            }
            UNLOCK();

            // Now we can stop the USB clock
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("PDD - SUSPEND\r\n")));
            timeout = INFINITE;
            continue;
        }

PROCESS_INTERRUPT:
        LOCK();
        RETAILMSG(DEBUG_LOG_USBCV, (L"\r\nHW!\r\n"));
        if (pPdd->bUSBCoreClk == FALSE)
        {
            BSPUSBClockDisable(FALSE);
            pPdd->bUSBCoreClk = TRUE;
        }

        if(!pPdd->bIsMX31TO2)
        {
            if (pPdd->bUSBPanicMode == FALSE)
            {
                DDKClockEnablePanicMode();
                pPdd->bUSBPanicMode = TRUE;
                DEBUGMSG(ZONE_INTERRUPTS, (TEXT("DDKClockEnablePanicMode\r\n")));
                DEBUGMSG(ZONE_INTERRUPTS, (TEXT("WakeUp - PORTSC: 0x%x\r\n"), INREG32(&pUSBDRegs->OTG.PORTSC[0])));
            }
        }

        if (INREG32(&pUSBDRegs->OTG.PORTSC[0]) & 0x800000)
        {
            RETAILMSG(DEBUG_LOG_USBCV, (L"PHCD reopen\r\n"));
            // Clear the PHCD
            CLRREG32(&pUSBDRegs->OTG.PORTSC[0], INREG32(&pUSBDRegs->OTG.PORTSC[0]) & 0x800000); //PORTSC_PHCD 0x800000
            // Force Resume bit
            OUTREG32(&pUSBDRegs->OTG.PORTSC[0], INREG32(&pUSBDRegs->OTG.PORTSC[0]) | 0x40); //PORTSC_FPR 0x40
            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("Force resume\r\n")));
        }
        UNLOCK();

#ifdef USBCV_FIX
       //081114 change the location of InterruptDone, pervent null HW interrupt
#else
        RETAILMSG(FALSE, (L"Pre INTDone\r\n"));
        InterruptDone(pPdd->sysIntr);
#endif

        CeLogMsg(_T("Intr:Signalled"));

        {
            USB_CTRL_T ctrl;
            temp = (DWORD *)&ctrl;
            *temp = INREG32(&pUSBDRegs->USB_CTRL);

            if (ctrl.OWIR == 1)
            {
                RETAILMSG(DEBUG_LOG_USBCV, (L"Re-Run\r\n"));
                *temp = 0;
                ctrl.OWIE = 1;
                CLRREG32(&pUSBDRegs->USB_CTRL, *temp);
                // We move to here since only when we have wakeup should we
                // set the Run bit of USB Controller again.
                USBControllerRun(pUSBDRegs, TRUE);
            }
        }
        // Exit thread when we are asked so...
        if (pPdd->exitIntrThread)
        {
            break;
        }

        if ( pPdd->bEnterTestMode )
        {
            LOCK();
            SetDeviceTestMode( pPdd, pPdd->iTestMode );

            /* clear whatever interrupt there was */
            temp=(DWORD *)&source;
            *temp = INREG32(&pUSBDRegs->OTG.USBSTS);

            // Clear source bits
            OUTREG32(&pUSBDRegs->OTG.USBSTS, *temp);

            /*
             * When device is in test mode, there is no longer any more
             * normal operation and we could just busy-stall here, since
             * exit from the mode is via power-off.
             */
            pPdd->exitIntrThread = TRUE;
            UNLOCK();
            break;
        }

        if (pPdd->m_CurSelfPowerState == D3 || pPdd->m_CurSelfPowerState == D4)
        {
            RETAILMSG(DEBUG_LOG_USBCV, (L"Power D2\r\n"));
            pPdd->m_CurSelfPowerState = D2;
            UpdateDevicePower(pPdd);
        }

        // Get interrupt source.
        temp=(DWORD *)&source;
        *temp = INREG32(&pUSBDRegs->OTG.USBSTS);

#ifdef USBCV_FIX
        tOtgsc = (DWORD*)&otgsc;
        *tOtgsc = INREG32(&pUSBDRegs->OTG.OTGSC);
#endif

        for (;;)
        {
            // Clear the reg
            RETAILMSG(FALSE, (L"USBSTS %x\r\n", *temp));

            OUTREG32(&pUSBDRegs->OTG.USBSTS, *temp);

#ifdef USBCV_FIX
            RETAILMSG(DEBUG_LOG_USBCV, (L"clear otg status with %x\r\n", *tOtgsc));
            OUTREG32(&pUSBDRegs->OTG.OTGSC, *tOtgsc);
            RETAILMSG(DEBUG_LOG_USBCV, (L"otg is now %x\r\n", INREG32(&pUSBDRegs->OTG.OTGSC)));

            {
                if (otgsc.BSVIS == 1)
                {
                    RETAILMSG(DEBUG_LOG_USBCV, (L"Find Disconnect\r\n"));
                    RETAILMSG(DEBUG_LOG_USBCV, (L"process disconnect\r\n"));
                    pPdd->devState=0;

                    LOCK();
                    pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
                    UNLOCK();

                    // SetSelfPowerState to D4
                    pPdd->m_CurSelfPowerState = D4; // Do we need set to D3 as wake up source?
                    UpdateDevicePower(pPdd);

                    OUTREG32(&pPdd->pUSBDRegs->OTG.T_154H.USBADR, 0);
                    if ((pPdd->devState == 0) && (pPdd->IsOTGSupport == 0))
                    {
                        timeout = IDLE_TIMEOUT;
                    }
                    break;
                }
            }
#endif

            if (source.URI)
            {
                RETAILMSG(DEBUG_LOG_USBCV, (L"USB RESET\r\n"));
                HandleUSBReset(pPdd);
            }

            if (source.SLI)
            {
                CeLogMsg(_T("Intr:SLI"));
                RETAILMSG(DEBUG_LOG_USBCV, (L"USB SLEEP\r\n"));
                LOCK();
                DevStatEvent(pPdd); // Handle device state change
                RETAILMSG(DEBUG_LOG_USBCV, (L"Go here, %d\r\n", pPdd->devState));
                if ((pPdd->devState == 0) && (pPdd->IsOTGSupport == 0))
                {
                    timeout = IDLE_TIMEOUT;
                }
                UNLOCK();
            }

            // USB Interrupt
            if (source.UI)
            {
                CeLogMsg(_T("Intr:UI SETUPSTAT %x ENDPTCOMP %x"),
                        INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT),
                        INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE) );

                RETAILMSG(DEBUG_LOG_USBCV, (L"USB UI\r\n"));
                while (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT) || INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE))
                {
                    if (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT))
                    {
#ifdef USBCV_FIX
                        ClearUsbstsUI(pUSBDRegs);
#endif
                        DEBUGMSG(ZONE_INTERRUPTS, (L"SetupEvent ...\r\n"));
                        RETAILMSG(DEBUG_LOG_USBCV, (L"Setup Packet\r\n"));
                        if (pPdd->StatusOutPrimed)
                        {
                            RETAILMSG(DEBUG_LOG_USBCV, (L"Nak here?\r\n"));
                                pPdd->StatusOutPrimed = FALSE;
                                OUTREG32(&pUSBDRegs->OTG.ENDPTNAK, 0x10001);
                                OUTREG32(&pUSBDRegs->OTG.ENDPTNAKEN, 0x0);
                        }
                        SetupEvent(pPdd);  // locks internally, and checks setupstat internally
                    }
                    if (INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE))
                    {
                        RETAILMSG(DEBUG_LOG_USBCV, (L"Other Packet\r\n"));
                        for (i = 0; i < USBD_EP_COUNT; i++)
                        {
                            // check endpoint sees if endptcomplete bit is set
                            // for individual endpoint i.
                            CheckEndpoint(pPdd, i );  // locks internally, and
                                                      // checks regs internally
                                                      // and clears the
                                                      // endptcomplete bit for
                                                      // that endpoint.
                        }
                    }
                }
            }

#ifdef USBCV_FIX
#else
            //The NAK interrupt is enabled to workaround the issue where the Device controller does not ACK
            //the repeated status stages but NAKs them.
            if (source.NAKI)
            {
                // Clear the NAK registers
                OUTREG32(&pUSBDRegs->OTG.ENDPTNAK, 0x10001);
                OUTREG32(&pUSBDRegs->OTG.ENDPTNAKEN, 0x0);

                UfnPdd_StallEndpoint(pPdd, 0);
            }
#endif

            *temp = INREG32(&pUSBDRegs->OTG.USBSTS);

#ifdef USBCV_FIX
            *tOtgsc = INREG32(&pUSBDRegs->OTG.OTGSC);
            //mask usbsts
            if (((*temp & USBD_IRQ_MASK) ==0 ) && ((*tOtgsc & USBD_OTGSC_IRQ_MASK) == 0))
#else
            if ((*temp & 0xffffff6f)==0)
#endif
            {
                RETAILMSG(FALSE, (L"NOINT\r\n"));
                CeLogMsg(_T("Intr:Done"));
                DEBUGMSG(ZONE_INTERRUPTS, (L"Break \r\n"));
                //CheckForEventLost(pPdd,k);

                break;
            }
            RETAILMSG(DEBUG_LOG_USBCV, (L"Next Round with usbsts %x, otgsc %x\r\n", *temp, *tOtgsc));
        }

#ifdef USBCV_FIX
        // 081114 we move this here
        RETAILMSG(DEBUG_LOG_USBCV, (L"Post IntDone\r\n"));
        InterruptDone(pPdd->sysIntr);
#endif
        if (pPdd->bResume)
        {
            RETAILMSG(DEBUG_LOG_USBCV, (L"Resume\r\n"));
            CeLogMsg(_T("Resume"));

            DEBUGMSG(ZONE_FUNCTION, (TEXT("+UFN_DETACH called\r\n")));
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
            DEBUGMSG(ZONE_FUNCTION, (TEXT("-UFN_DETACH called\r\n")));

            LOCK();
            // Don't process other changes (we are disconnected)
            pPdd->devState=0;
            pPdd->bResume = FALSE;
            // SetSelfPowerState to D4
            pPdd->m_CurSelfPowerState = D4; // Do we need set to D3 as wake up source?
            UpdateDevicePower(pPdd);

            OUTREG32(&pPdd->pUSBDRegs->OTG.T_154H.USBADR, 0);

            UNLOCK();

            if (pPdd->IsOTGSupport)
            {
                InterruptDisable(pPdd->sysIntr);
                // wait for client function class and make sure everything is
                // disconnected this is not a very good design but since we
                // need to make sure we can get proper disconnect before we
                // switch back to XVR, we have to do that especially on
                DEBUGMSG(ZONE_FUNCTION, (TEXT("Leave Client back to XVR mode\r\n")));
                Sleep(500);
                SetEvent(pPdd->hXcvrEvent);
                goto XCVR_SIG;
            }
        }

        // Handle detach condition for OTG
        LOCK();
        if (pPdd->IsOTGSupport && pPdd->devState == 0)
        {
            USB_PORTSC_T state;
            USB_OTGSC_T otgsc;
            DWORD *temp2;
            DWORD *temp3;

            RETAILMSG(DEBUG_LOG_USBCV, (L"detach\r\n"));
            temp2 = (DWORD *)&state;
            *temp2 = INREG32(&pUSBDRegs->OTG.PORTSC[0]);

            temp3 = (DWORD *)&otgsc;
            *temp3 = INREG32(&pUSBDRegs->OTG.OTGSC);

            CeLogMsg(_T("Detach: PORTSC 0x%x"), *temp2);

            DEBUGMSG(ZONE_INTERRUPTS, (TEXT("*temp3 0x%x \r\n"), *temp3));

            RETAILMSG(DEBUG_LOG_USBCV, (L"susp %d, ID %d\r\n", state.SUSP, otgsc.ID));
#ifdef USBCV_FIX
            if (otgsc.AVV == 0)
#else
            if ((state.SUSP == 1) || (otgsc.ID == 0))
#endif
            {
                DEBUGMSG(ZONE_INTERRUPTS, (TEXT("Device Detach, with susp(%d), id(%d)\r\n"), state.SUSP, otgsc.ID));
                if (otgsc.IDIS)
                {
                    // clear the register. since other is not used, we can
                    // simply use the SETREG32 instead of OUTREG32
                    *temp3 = 0;
                    otgsc.IDIS = 1;
                    SETREG32(&pUSBDRegs->OTG.OTGSC, *temp3);
                }
                UNLOCK();

                InterruptDisable(pPdd->sysIntr);
                RETAILMSG(DEBUG_LOG_USBCV, (L"d -> x\r\n"));
                SetEvent(pPdd->hXcvrEvent);
                goto XCVR_SIG;
            }
        }

        if ((pPdd->IsOTGSupport == 0) && (pPdd->devState == 0))
        {
            timeout = IDLE_TIMEOUT;
        }

        UNLOCK();
    }

    if (pPdd->bEnterTestMode)
    {
        DEBUGMSG(ZONE_PDD, (TEXT("USBOTG Client Test Mode set and loop forever\r\n")));
        pPdd->bEnterTestMode = FALSE;
        for (;;);
    }

    return ERROR_SUCCESS;
}


//-----------------------------------------------------------------
//
//  Function: BuildTdBufferList
//
//  Given a mapped endpoint and a TransferDescriptor and a length
//  of data to put into it, build the list of buffer pointers from
//  previously locked page data (including current page). ep is
//  current endpoint, pTd points to the TD being built in uncached
//  mem.
//
//  Parameter:
//
//      [IN] ep - endpoint to be processed
//      [IN] pTd - pointer to transfer descriptor
//      [IN] len - transfer length
//
//  Return:
//
//      NULL
//
//-------------------------------------------------------------------
static void BuildTdBufferList( USBFN_EP *ep, PUSBD_dTD_R_T pTd, DWORD len, STransfer *pTransfer, BOOL InTransfer, BOOL bFirstTimeThrough )
{
    DWORD dwOffset;
    DWORD dwPageIdx;
    DWORD BuffLen;

    BuffLen = len;

    if ( len > 0 )
    {
        dwPageIdx = ep->dwNextPageIdx;
        DEBUGCHK( dwPageIdx < ep->dwNumPages );

        pTd->bp0 = (ep->pdwPageList[dwPageIdx++] << UserKInfo[KINX_PFN_SHIFT]) >> 12;

        // get current buffer pointer's offset into first page
        pTd->curr_off = dwOffset = ((DWORD)ep->pMappedBufPtr) & (UserKInfo[KINX_PAGESIZE] - 1);

        if(!InTransfer)
        {
            // If this is the first IssueTransfer for this transfer, onlu then check for the Offset alignment
            if(bFirstTimeThrough)
            {
                // Need to check for both Offset alignment and length if the length is very less and ends in one page
                if ((dwOffset % 32) != 0 || ((len < (MAX_SIZE_PER_BP - dwOffset)) && ((len % 32) != 0)))
                {
                    //RETAILMSG(1,(_T("Inside UNCHACHED for bp0\r\n")));
                    pTd->bp0 = phyAddr_TD1.LowPart >> 12;

                    // get current buffer pointer's offset into first page
                    pTd->curr_off = dwOffset;
                }
            }
            else  // If this is not the first IssueTransfer for this transfer
            {
                // Check for the lentgh alignment if the length ends at this page itself
                if ((len < (MAX_SIZE_PER_BP - dwOffset)) && ((len % 32) != 0))
                {
                    //RETAILMSG(1,(_T("Inside UNCHACHED for bp0\r\n")));
                    pTd->bp0 = phyAddr_TD2.LowPart >> 12;

                    // get current buffer pointer's offset into first page
                    pTd->curr_off = 0;
                }
            }
        }

        /* the first buffer page will contain either one MAX_SIZE_PER_BP or less (if there was an offset) */
        if ( len > (MAX_SIZE_PER_BP - dwOffset) )
            len -= (MAX_SIZE_PER_BP - dwOffset);
        else
            len = 0;   // first bp contains all the data

        // continue to buffer page 1
        if ( len > 0 )
        {
            DEBUGCHK( dwPageIdx < ep->dwNumPages );

            pTd->bp1 = (ep->pdwPageList[dwPageIdx++] << UserKInfo[KINX_PFN_SHIFT])>>12;

            // each of the following pages will start at offset 0
            if ( len > MAX_SIZE_PER_BP )
                len -= MAX_SIZE_PER_BP;
            else
            {
                if(!InTransfer)
                {
                    if ((len % 32) != 0)
                    {
                        //RETAILMSG(1,(_T("Inside UNCHACHED for bp1\r\n")));
                        pTd->bp1 = phyAddr_TD2.LowPart >> 12;
                    }
                }
                len = 0;
            }

            // continue to buffer page 2
            if ( len > 0 )
            {
                DEBUGCHK( dwPageIdx < ep->dwNumPages );

                pTd->bp2 = (ep->pdwPageList[dwPageIdx++] << UserKInfo[KINX_PFN_SHIFT])>>12;

                if ( len > MAX_SIZE_PER_BP )
                    len -= MAX_SIZE_PER_BP;
                else
                {
                    if(!InTransfer)
                    {
                        if ((len % 32) != 0)
                        {
                            //RETAILMSG(1,(_T("Inside UNCHACHED for bp2\r\n")));
                            pTd->bp2 = phyAddr_TD2.LowPart >> 12;
                        }
                    }
                    len = 0;
                }

                // continue to buffer page 3
                if ( len > 0 )
                {
                    DEBUGCHK( dwPageIdx < ep->dwNumPages );

                    pTd->bp3 = (ep->pdwPageList[dwPageIdx++] << UserKInfo[KINX_PFN_SHIFT])>>12;

                    if ( len > MAX_SIZE_PER_BP )
                        len -= MAX_SIZE_PER_BP;
                    else
                    {
                        if(!InTransfer)
                        {
                            if ((len % 32) != 0)
                            {
                                //RETAILMSG(1,(_T("Inside UNCHACHED for bp3\r\n")));
                                pTd->bp3 = phyAddr_TD2.LowPart >> 12;
                            }
                        }

                        len = 0;
                    }

                    // continue to buffer page 4
                    if (  len > 0 )
                    {
                        DEBUGCHK( dwPageIdx < ep->dwNumPages );

                        pTd->bp4 = (ep->pdwPageList[dwPageIdx] << UserKInfo[KINX_PFN_SHIFT])>>12;

                        // Need to check if the there is still data to come in this transfer(if data > 16k)
                        // so should not check for alignment if some more data is to come for this transfer
                        if(!InTransfer && !(BuffLen == MAX_SIZE_PER_TD && pTransfer->cbBuffer > MAX_SIZE_PER_TD) )
                        {
                            if ((len % 32) != 0)
                            {
                                //RETAILMSG(1,(_T("Inside UNCHACHED for bp4\r\n")));
                                pTd->bp4 = phyAddr_TD2.LowPart >> 12;
                            }
                        }
                        /* whenever we extend into this fifth BP, we are either completely finished
                         * with all our data, or next time we should start with the same page (from
                         * the current offset
                         */
                    }
                }
            }
        }

        ep->dwNextPageIdx = dwPageIdx;
    }
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IssueTransfer
//
//  This is called by MDD to issue transfer to USB host.
//
//  Parameter:
//
//      pPddContext - Pointer to USBFN_PDD
//      endPoint    - endpoint to be transferred
//      pTransfer   - pointer to STransfer
//
//  Return:
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_IssueTransfer(
                                  VOID *pPddContext,
                                  DWORD endPoint,
                                  STransfer *pTransfer
                                 )
{
    USBFN_PDD *pPdd = pPddContext;
    DWORD epNum;
    PUSBD_dTD_R_T pTd;
    PUSBD_dQH_T pQh;
    DWORD len=0;
    DWORD mapped_epNum;
    BOOL  bOk;
    BOOL  bFirstTimeThrough;

    LOCK();

    if(!pTransfer)
    {
        UNLOCK();
        return ERROR_SUCCESS;
    }
    // Save transfer for interrupt thread
    // Select EP
    epNum = USBD_EP_NUM & endPoint;

    mapped_epNum = epNum;
    if ((pTransfer && TRANSFER_IS_IN(pTransfer)) && (epNum == 0))
    {
        /*
         * For IN EP0, we map to ep0 to the last EP instead.
         * This isn't strictly necessary but was introduced early in
         * development due to restrictions at that time.
         * IN  EP0 => USBD_EP_COUNT-1
         * OUT EP0 => 0
         */
        mapped_epNum = USBD_EP_COUNT-1;
    }

    CeLogMsg(_T("IssueTransfer: %s with %d bytes"),
        pTransfer ? ( TRANSFER_IS_IN(pTransfer) ? _T("IN") : _T("OUT") ) :
                      _T("NULL"),
        pTransfer ? pTransfer->cbBuffer : 0 );

    DEBUGMSG(ZONE_TRANSFER, (_T("IssTran (%d): %s (pTransfer=0x%x %d bytes) %d remain. ")
                  _T("err=0x%x, bPrimed=0x%x\r\n"),
                  epNum, pTransfer ? ( TRANSFER_IS_IN(pTransfer) ? _T("IN") :
                                     _T("OUT") ) : _T("NULL"),
                  pTransfer,
                  pTransfer ? pTransfer->cbBuffer : -1,
                  pTransfer ? pTransfer->cbBuffer-pTransfer->cbTransferred : -1,
                  pTransfer? pTransfer->dwUsbError:0,
                  pPdd->qhbuffer->bPrimed[mapped_epNum]));

    if (pPdd->ep[mapped_epNum]->pTransfer &&
        pPdd->ep[mapped_epNum]->pTransfer == pTransfer )
    {
        if ( pTransfer->dwUsbError!=UFN_MORE_DATA )
        {
            UNLOCK();
            bFirstTimeThrough = FALSE;
            return ERROR_SUCCESS;
        }

        bFirstTimeThrough = FALSE;
    }
    else
    {
        bFirstTimeThrough = TRUE;

    }

    DEBUGMSG(ZONE_TRANSFER, (TEXT("UfnPdd_IssueTransfer first time(0x%x)\r\n"),
                  bFirstTimeThrough));

    if (!TRANSFER_IS_IN(pTransfer)||epNum)
    {
        pPdd->ep[epNum]->pTransfer = pTransfer;
    }
    else
    {
        // IN transaction for EP 0.
        pPdd->ep[USBD_EP_COUNT-1]->pTransfer = pTransfer;
    }

    if (pTransfer->pvBuffer == NULL ) // Sync Length with buffer.
    {
        pTransfer->cbBuffer = 0 ;
    }

    if (bFirstTimeThrough)
    {
        /*
         * Issue Transfer can be called several times per transfer.
         * This is the first time we've called this function for this transfer
         */
        pTransfer->cbTransferred=0;  // so nothing transferred yet

        pPdd->ep[mapped_epNum]->dwNextPageIdx = 0;

#ifdef USBCV_MSC
        if ((pTransfer->pvBuffer) && (pTransfer->cbBuffer))
#else
        if ( pTransfer->pvBuffer )
#endif
        {
            /* user provided a buffer so we need to lock it and get the
                      * physical pages.
                      */

            //ufnpdd_IssueTransfer is called from the MDD and is expected to have the buffer
            //already marashalled by the MDD. So removing the Marshalling code.
            pPdd->ep[mapped_epNum]->pMappedBufPtr = pTransfer->pvBuffer;

            bOk = FALSE;
            if ( pPdd->ep[mapped_epNum]->pMappedBufPtr == NULL )
            {
                // this is a security violation
                DEBUGMSG(ZONE_ERROR,
                    (_T("UfnPdd_IssueTransfer: Failed to map buffer pointer ")
                     _T("(0x%x, %d bytes) to caller\r\n"),
                    pTransfer->pvBuffer,
                    pTransfer->cbBuffer
                    ));
            }
            else
            {
                if ( pTransfer->cbBuffer > 0 )
                {
                    bOk = LockPages(pPdd->ep[mapped_epNum]->pMappedBufPtr,
                                    pTransfer->cbBuffer,
                                    pPdd->ep[mapped_epNum]->pdwPageList,
                                    TRANSFER_IS_IN(pTransfer) ? LOCKFLAG_READ :
                                                                LOCKFLAG_WRITE);
                }
            }

            // Could check direction of transfer and just lock read, but
            // do this for now.

            DEBUGCHK(bOk || (pTransfer->cbBuffer == 0));

            if ( !bOk )
            {
                DWORD dwGLE;

                RETAILMSG(FALSE, (L"Lock Page not OK\r\n"));

                if ( pTransfer->cbBuffer > 0 )
                {
                    dwGLE = GetLastError();
                    DEBUGMSG(ZONE_ERROR, (_T("UfnPdd_IssueTransfer: Buffer ")
                                          _T("Page Lock Failed. Error:\r\n"),
                                          dwGLE));
                    DEBUGMSG(ZONE_ERROR, (_T("UfnPdd: Page translation failed. Error: ")
                                  _T("%d\r\n"), dwGLE));

                    pPdd->ep[mapped_epNum]->pMappedBufPtr = NULL;
                    pPdd->ep[mapped_epNum]->bPagesLocked = FALSE;
                    pPdd->ep[mapped_epNum]->pTransfer = NULL;
                    pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;

                    UNLOCK();
                    pPdd->pfnNotify(pPdd->pMddContext,
                                    UFN_MSG_TRANSFER_COMPLETE,
                                    (DWORD)pTransfer);

                    return ERROR_SUCCESS;  // should be an error code?
                }

                // Otherwise, it's a normal situation for null transfer on
                // user-level.
            }
            else
            {
                // Lock pages success.
                pPdd->ep[mapped_epNum]->bPagesLocked =
                    (pTransfer->cbBuffer > 0);
                pPdd->ep[mapped_epNum]->dwNextPageIdx = 0;
                pPdd->ep[mapped_epNum]->dwNumPages =
                    ADDRESS_AND_SIZE_TO_SPAN_PAGES(pTransfer->pvBuffer,
                                                   pTransfer->cbBuffer);
            }
        }
#ifdef USBCV_MSC
        else if (pTransfer->cbBuffer)
        {
            //This is a zero length transfer
            pPdd->ep[mapped_epNum]->pMappedBufPtr = NULL;
            pPdd->ep[mapped_epNum]->bPagesLocked = 0;
            pPdd->ep[mapped_epNum]->dwNextPageIdx = 0;
            pPdd->ep[mapped_epNum]->dwNumPages = 0;
        }
#endif
    }

    len=pTransfer->cbBuffer-pTransfer->cbTransferred;

    /* We use a one TD for each portion of the transfer, for now.
     * calculate how much we can transfer in this portion.
     * There are 5 page pointers per TD, and first (or last)
     * might contain less than a full page. If we use a round
     * figure for len then we end up with even number of maxsize
     * packets per TD.
     */

    if ( len > MAX_SIZE_PER_TD )
        len = MAX_SIZE_PER_TD;

    DEBUGCHK(pTransfer->dwUsbError == UFN_NOT_COMPLETE_ERROR);

    if (TRANSFER_IS_IN(pTransfer))
    {
        pPdd->ep[mapped_epNum]->fZeroLengthNeeded = (pTransfer->cbBuffer==0);
        pTd=&(pPdd->qhbuffer->td[epNum*2+1]);
        pQh=&(pPdd->qhbuffer->qh[epNum*2+1]);

        if (epNum==0)
        {
            pPdd->NSend0ACK = TRUE;     // TRANSFER_IS_IN means it is a IN token transfer
                                        // from periphreal viewpoint, it is a Transmit packet
                                        // Transmit on EP0 need a host zero packet for acknowledge
                                        // i.e. "SETUP - IN - OUT0" such as get descriptor
        }
    }
    else
    {
        pPdd->ep[mapped_epNum]->fZeroLengthNeeded = (pTransfer->cbBuffer==0);
        pTd=&(pPdd->qhbuffer->td[epNum*2]);
        pQh=&(pPdd->qhbuffer->qh[epNum*2]);
    }

    memset(pTd, 0, sizeof(USBD_dTD_T));
    pTd->T=1;
    pTd->next_dtd=0xDEAD;
    pTd->tb=len;    // Assume cbBuffer <0x1000
    pTd->ioc=1;
    pTd->status=0x80;            // Active

    /* Point up to 5 buffer pointers at the current pages
     * we have one static TD right now.
     */

    BuildTdBufferList( pPdd->ep[mapped_epNum], pTd, len, pTransfer, TRANSFER_IS_IN(pTransfer), bFirstTimeThrough );

    // Assume only one dTD for one endpoint(except 0) at anytime.
    RETAILMSG(DEBUG_LOG_USBCV, (L"Begin Transfer len %d\r\n", len));
    {
        USB_ENDPTPRIME_T edptprime;
        DWORD * temp=(DWORD * )&edptprime;

        *temp=0;
        if (TRANSFER_IS_IN(pTransfer))
        {
            edptprime.PETB=1<<epNum;
        }
        else
        {
            edptprime.PERB=1<<epNum;
        }


        while (INREG32(&pPdd->pUSBDRegs->OTG.ENDPTPRIME))
        {
            ;    // Wait until the last prime is finished
        }

        if((INREG32(&pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE)&(*temp))!=0)
        {
            RETAILMSG(DEBUG_LOG_USBCV, (L"need we clear this in priming ?\r\n"));
            OUTREG32(&(pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE), *temp);
        }

        memset(pQh, 0, sizeof(USBD_dQH_T));
        pQh->ios = 1;
        pQh->dtd.next_dtd=(DescriptorPhy(pPdd, (DWORD)pTd)>>5);
        pQh->zlt=(pPdd->ep[mapped_epNum]->fZeroLengthNeeded)?0:1;
        pQh->mpl=pPdd->ep[mapped_epNum]->maxPacketSize;

        CacheSync(CACHE_SYNC_DISCARD);

        // start the prime
        OUTREG32(&pPdd->pUSBDRegs->OTG.ENDPTPRIME,*temp);

        while (INREG32(&pPdd->pUSBDRegs->OTG.ENDPTPRIME)){}

        if(INREG32(&pPdd->pUSBDRegs->OTG.ENDPTSETUPSTAT) && (mapped_epNum == 0))
        {
            DEBUGMSG(ZONE_TRANSFER,(_T("UfnPdd: new setup request detected\r\n")));

            RETAILMSG(DEBUG_LOG_USBCV, (L"Setup Here?\r\n"));

            // SHOULD result in an immediate interrupt to check setup
            // packet status.
            SetEvent(pPdd->hIntrEvent);

            // unmap pages
            if ( pPdd->ep[mapped_epNum]->bPagesLocked )
            {
               UnlockPages(pPdd->ep[mapped_epNum]->pMappedBufPtr,
                           pTransfer->cbBuffer);
            }

            pPdd->ep[mapped_epNum]->pMappedBufPtr = NULL;
            pPdd->ep[mapped_epNum]->bPagesLocked = FALSE;
            pPdd->ep[mapped_epNum]->pTransfer = NULL;
            pTransfer->dwUsbError = UFN_CANCELED_ERROR;

            UNLOCK();

            DEBUGMSG(ZONE_TRANSFER, (TEXT("IssueTransfer SETUPSTAT received on %s at ")
                          TEXT("%d\r\n"),
                          TRANSFER_IS_IN(pTransfer) ? TEXT("IN") : TEXT("OUT"),
                          __LINE__));

            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE,
                            (DWORD)pTransfer);

            return ERROR_TIMEOUT;  // should be an error code?
        }

        if ((INREG32(&pPdd->pUSBDRegs->OTG.ENDPTSTATUS)&(*temp))==0)
        {
            RETAILMSG(DEBUG_LOG_USBCV, (L"Buf Not Ready\r\n"));
            if ( (INREG32(&pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE)&(*temp))==0 )
            {
                DEBUGMSG(ZONE_ERROR, (L"############# Prime Failed for ep=%d "
                              L"STAT:0x%x WITH:0x%x try again??\r\n",
                              epNum,
                              INREG32(&pPdd->pUSBDRegs->OTG.ENDPTSTATUS),
                              *temp));
            }
        }
    }

    pPdd->qhbuffer->bPrimed[mapped_epNum] = TRUE;

    UNLOCK();
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  FlushEndpoint
//
//  This function is to flush the endpoint and any primed buffer would be clear.
//
//  Parameter:
//
//      pPddContext - Pointer to USBFN_PDD
//      epNum - endpoint to be transferred
//      bIsReceive - TRUE : Receive Buffer,  FALSE : Transmit Buffer
//
//  Return:
//      NULL
//
//--------------------------------------------------------------------------------
static void FlushEndpoint( USBFN_PDD *pPdd, DWORD epNum, BOOL bIsReceive )
{
    USB_ENDPTFLUSH_T edptflush;
    DWORD *pFlush=(DWORD * )&edptflush;

    USB_ENDPTSTAT_T stat;
    DWORD *pdwStat = (DWORD*)&stat;

    *pFlush = 0;
    if ( bIsReceive )
    {
        edptflush.FERB=1<<epNum;
    }
    else
    {
        edptflush.FETB=1<<epNum;
    }
    // flush the endpoint
    // check whether it's primed for this transaction
    *pdwStat = INREG32(&pPdd->pUSBDRegs->OTG.ENDPTSTATUS);
    if ( (edptflush.FETB && (stat.ETBR & (1<<epNum))) ||
         (edptflush.FERB && (stat.ERBR & (1<<epNum))) )
    {
        OUTREG32(&pPdd->pUSBDRegs->OTG.ENDPTFLUSH,*pFlush);
        while (INREG32(&pPdd->pUSBDRegs->OTG.ENDPTFLUSH) & *pFlush)
        {
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_AbortTransfer
//
//  This is called by MDD to abort any transfer
//
//  Parameters:
//
//      pPddContext - Pointer to USBFN_PDD
//      endPoint - end point to which the transaction to be aborted
//      pTransfer - pointer to STransfer
//
//  Return:
//      ERROR_SUCCESS
//
//--------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_AbortTransfer(
                                  VOID *pPddContext,
                                  DWORD endPoint,
                                  STransfer *pTransfer
                                  )
{
    USBFN_PDD *pPdd = pPddContext;
    DWORD epNum, mappedEp;

    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd_AbortTransfer+ (EP= %d)\r\n", endPoint));

    CeLogMsg(_T("AbortTransfer (%s EP%d)"),
             pTransfer ? ( TRANSFER_IS_IN(pTransfer) ? _T("IN") :
                           _T("OUT") ) : _T("NULL"), endPoint );

    // Select EP
    epNum = USBD_EP_NUM & endPoint;

    if ( !pTransfer )
    {
        DEBUGMSG(ZONE_ERROR, (L"UsbFnPdd_AbortTransfer: pTransfer is NULL "
                              L"on EP%d!\r\n", epNum));

        return ERROR_INVALID_PARAMETER;
    }

    if ( epNum == 0 )
    {
        mappedEp = TRANSFER_IS_IN(pTransfer) ? USBD_EP_COUNT - 1 : 0;
    }
    else
        mappedEp = epNum;

    // make sure the EP currently is running the transfer we're talking about

    LOCK();

    if ( pPdd->ep[mappedEp]->pTransfer == pTransfer )
    {
        DEBUGMSG(ZONE_TRANSFER,(TEXT("########### AbortTransfer(%d) Dev state %d\r\n"), mappedEp, pPdd->devState) );
        FlushEndpoint( pPdd, mappedEp, TRANSFER_IS_OUT(pTransfer) );

        if ( pPdd->ep[mappedEp]->bPagesLocked )
        {
            DEBUGCHK( pPdd->ep[mappedEp]->pMappedBufPtr != NULL );
            UnlockPages(pPdd->ep[mappedEp]->pMappedBufPtr,
                        pTransfer->cbBuffer);
            pPdd->ep[mappedEp]->bPagesLocked = 0;
        }

        // Finish transfer
        pPdd->ep[mappedEp]->pTransfer = NULL;

        pTransfer->dwUsbError = UFN_CANCELED_ERROR;
        UNLOCK();

        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE,
                        (DWORD)pTransfer);
    }
    else
    {
        DEBUGMSG(ZONE_TRANSFER, (TEXT("Abort(%d) skipped (Tran 0x%x != ")
                            TEXT("0x%x)\r\n"),
                            mappedEp,
                            pPdd->ep[mappedEp]->pTransfer,
                            pTransfer ) );

        pTransfer->dwUsbError = UFN_CANCELED_ERROR;

        UNLOCK();

        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE,
                        (DWORD)pTransfer);
    }

    DEBUGMSG(ZONE_TRANSFER, (L"UsbFnPdd_AbortTransfer- (EP= %d)\r\n", endPoint));

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_StallEndpoint
//
//  This function is called by MDD to set end point to stall mode (halted).
//
//  Parameters:
//      pPddContext - Pointer to USBFN_PDD
//      endPoint - endpoint to stall
//
//  Return:
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_StallEndpoint(VOID *pPddContext, DWORD endPoint)
{
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    USB_ENDPTCTRL_T edptctrl;
    DWORD epNum, *temp=(DWORD *)&edptctrl;

    DEBUGMSG(ZONE_PDD, (_T("UsbFnPdd_StallEndpoint %d\r\n"), endPoint));

    LOCK();

    *temp=0;
    if (endPoint == 0)
    {
        edptctrl.TXS=edptctrl.RXS=1;
        // Stall next EP0 transaction
        SETREG32(&pUSBDRegs->OTG.ENDPTCTRL0, *temp);
    }
    else
    {
        // Select EP
        epNum = USBD_EP_NUM & endPoint;
        if (!pPdd->ep[epNum]->dirRx)
        {
            edptctrl.TXS=1;
        }
        else
        {
            edptctrl.RXS=1;
        }

        // Stall EP
        SETREG32(&pUSBDRegs->OTG.ENDPTCTRL[epNum-1], *temp);
    }

    UNLOCK();

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_ClearEndpointStall
//
//  This function is called by MDD to clear end point stall mode (halted).
//
//  Parameters:
//      pPddContext - Pointer to USBFN_PDD
//      endPoint - endpoint to stall
//
//  Return:
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_ClearEndpointStall(VOID *pPddContext, DWORD endPoint)
{
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    USB_ENDPTCTRL_T edptctrl;
    DWORD epNum, *temp=(DWORD *)&edptctrl;

    DEBUGMSG(ZONE_PDD, (_T("UsbFnPdd_ClearEndpoint %d\r\n"), endPoint));

    LOCK();
    *temp=0;
    if (endPoint == 0)
    {
        *temp=INREG32(&pUSBDRegs->OTG.ENDPTCTRL0);
        edptctrl.TXS=edptctrl.RXS=0;
        edptctrl.TXR=1; edptctrl.RXR=1;
        OUTREG32(&pUSBDRegs->OTG.ENDPTCTRL0, *temp);
    }
    else
    {
        // Select EP
        epNum = USBD_EP_NUM & endPoint;
        *temp=INREG32(&pUSBDRegs->OTG.ENDPTCTRL[epNum-1]);
        if (!pPdd->ep[epNum]->dirRx)
        {
            edptctrl.TXS=0;
            edptctrl.TXR=1;
        }
        else
        {
            edptctrl.RXR=1;
            edptctrl.RXS=0;
        }
        OUTREG32(&pUSBDRegs->OTG.ENDPTCTRL[epNum-1], *temp);
    }

    UNLOCK();
    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IsEndpointHalted
//
//  This function is to check if endpoint is halted
//
//  Parameters:
//      [IN]  pPddContext - Pointer to USBFN_PDD
//      [IN]  endPoint - endpoint to stall
//      [OUT] pHalted - pointer to indicate if endpoint is halted.
//
//  Return:
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------

DWORD WINAPI UfnPdd_IsEndpointHalted(
                                     VOID *pPddContext,
                                     DWORD endPoint,
                                     BOOL *pHalted
                                    )
{
    DWORD rc = ERROR_INVALID_FUNCTION;
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd_IsEndpointHalted %d\r\n", endPoint));

    // Endpoint can't be zero
    if (endPoint == 0)
    {
        goto clean;
    }

    // Select EP
    epNum = USBD_EP_NUM & endPoint;

    LOCK();
    // Is EP halted?
    {
        USB_ENDPTCTRL_T ctrl;
        DWORD *t=(DWORD*)&ctrl;
        *t=INREG32(&pUSBDRegs->OTG.ENDPTCTRL[epNum-1]);
        if (pPdd->ep[endPoint]->dirRx)
        {
            *pHalted=ctrl.RXS;
        }
        else
        {
            *pHalted =ctrl.TXS;
        }
    }
    UNLOCK();

    // Done
    rc = ERROR_SUCCESS;

clean:
    DEBUGMSG(ZONE_PDD, (L"-UsbFnPdd_IsEndpointHalted %d\r\n", endPoint));
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_SendControlStatusHandshake
//
//  Send the control status handshake.
//
//  Parameters:
//      [IN]  pPddContext - Pointer to USBFN_PDD
//      [IN]  endPoint - endpoint to stall
//
//  Return:
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_SendControlStatusHandshake(
    VOID *pPddContext, DWORD endPoint
    )
{
    USBFN_PDD *pPdd = pPddContext;

    CeLogMsg(_T("+SendCSH (EP%d)"), endPoint );

    LOCK();

    RETAILMSG(DEBUG_LOG_USBCV, (L"UfnPdd_SendControlStatusHandshake\r\n"));
    if ( pPdd->NSend0ACK )
    {
        DEBUGMSG(ZONE_TRANSFER,(_T("SendCSH OUT (%d)\r\n"), endPoint) );

        RETAILMSG(DEBUG_LOG_USBCV, (L"HandShake OUT\r\n"));

        PrimeQh0ForZeroTransfer(pPdd, FALSE );

        // just prime for zero length OUT (receive data)
        pPdd->NSend0ACK = FALSE;
        pPdd->StatusOutPrimed = TRUE;
    }
    else
    {
        DEBUGMSG(ZONE_TRANSFER,(_T("SendCSH IN (%d)\r\n"), endPoint) );

        RETAILMSG(DEBUG_LOG_USBCV, (L"HandShake IN\r\n"));

        PrimeQh0ForZeroTransfer(pPdd, TRUE );
    }

#ifdef USBCV_FIX
    RETAILMSG(DEBUG_LOG_USBCV, (L"what if we don't manual set system interrupt\r\n"));
#else
    SetEvent(pPdd->hIntrEvent);  // and trigger interrupt to process this
#endif

    CeLogMsg(_T("-SendCSH (EP%d)"), endPoint );

    UNLOCK();

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_InitiateRemoteWakeup
//
//  This function is to handle the RemoteWakeUp request from MDD
//
//  Parameters:
//      [IN]  pPddContext - Pointer to USBFN_PDD
//
//  Return:
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------

DWORD WINAPI UfnPdd_InitiateRemoteWakeup(VOID *pPddContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPddContext);

    DEBUGMSG(ZONE_PDD, (_T("UsbFnPdd_InitiateRemoteWakeup\r\n")));

    return ERROR_SUCCESS;
}
//------------------------------------------------------------------------------
//
//  Function:  SetPowerState
//
//  This function is to set the power state
//
//  Parameters:
//      [IN]  pPdd - Pointer to USBFN_PDD
//      [IN]  cpsNew - device power state
//
//  Return:
//      NULL
//
//------------------------------------------------------------------------------
void SetPowerState(USBFN_PDD *pPdd , CEDEVICE_POWER_STATE cpsNew )
{
    PREFAST_ASSERT(pPdd!=NULL) ;
    // Adjust cpsNew.
    DEBUGMSG(ZONE_PDD, (TEXT("SetPowerState = 0x%x\r\n"), cpsNew));
    if (cpsNew == PwrDeviceUnspecified)
    {
        pPdd->m_CurPMPowerState = PwrDeviceUnspecified;
        return;
    }

    // Only support D0 and D4
    if (cpsNew != pPdd->m_CurPMPowerState )
    {
        if (cpsNew != D4)
        {
            cpsNew = D0;
        }
        else
        {
            cpsNew = D4;
            //pPdd->devState=0;
            //pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
        }
    }
    if (cpsNew != pPdd->m_CurPMPowerState )
    {
        pPdd->m_CurPMPowerState = cpsNew;
        UpdateDevicePower( pPdd);
    }
    return;
}

//------------------------------------------------------------------------------
//
//  Function:  SetDeviceTestMode
//
//  This function puts device controller directly into one of the test modes
//
//  Parameter:
//     pPdd - Pointer to USBFN_PDD
//     iTestMode - Test mode to be set
//
//  Return:
//     TRUE - success;  FALSE - failure
//
//------------------------------------------------------------------------------
static BOOL SetDeviceTestMode(USBFN_PDD *pPdd, int iTestMode )
{
#ifndef USBFN_TEST_MODE_SUPPORT
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPdd);
    UNREFERENCED_PARAMETER(iTestMode);
#endif

#ifdef USBFN_TEST_MODE_SUPPORT

    UCHAR temp = 0;
    CSP_USB_REGS*pUSBDRegs = pPdd->pUSBDRegs;

    USB_PORTSC_T portsc;
    DWORD  *pTsc;

//The below modes require the nrzi and bit stuffing disabled and others we still normal mode.
    if((iTestMode == USB_TEST_MODE_J_STATE) ||
       (iTestMode == USB_TEST_MODE_K_STATE) ||
       (iTestMode == USB_TEST_MODE_SE0_NAK)
      )
    {
        temp = ISP1504_ReadReg((DWORD *)&pUSBDRegs->OTG.ULPI_VIEWPORT, ISP1504_FUNCTION_CTRL_RW);
        temp = temp &(~0x18); // Clear OpMode[] bits
        temp = temp | 0x10; // Set OpMode[] = 10 binary
        ISP1504_WriteReg((DWORD *)&pUSBDRegs->OTG.ULPI_VIEWPORT, ISP1504_FUNCTION_CTRL_S, temp);
    }

    pTsc  = (DWORD *)&portsc;

    /* The device controller set select bits are actually the same definitions
     * as in the USB spec for the SetFeature command, but have a switch just
     * in case the controller defines change.
     */
    *pTsc = INREG32(&pUSBDRegs->OTG.PORTSC[USBD_PORT_NUM]);

    switch ( iTestMode )
    {
        case USB_TEST_MODE_DISABLE:
            portsc.PTC = 0;
            break;
        case USB_TEST_MODE_J_STATE:
            portsc.PTC = 1;
            break;
        case USB_TEST_MODE_K_STATE:
            portsc.PTC = 2;
            break;
        case USB_TEST_MODE_SE0_NAK:
            portsc.PTC = 3;
            break;
        case USB_TEST_MODE_PACKET:
            portsc.PTC = 4;
            break;
        case USB_TEST_MODE_FORCE_ENABLE_HS:
            portsc.PTC = 5;
            break;
        case USB_TEST_MODE_FORCE_ENABLE_FS:
            portsc.PTC = 6;
            break;
        case USB_TEST_MODE_FORCE_ENABLE_LS:
            portsc.PTC = 7;
            break;
        default:
            return FALSE;
    }

    OUTREG32(&pUSBDRegs->OTG.PORTSC[USBD_PORT_NUM], *pTsc);

    // read back for debug
    *pTsc = INREG32(&pUSBDRegs->OTG.PORTSC[USBD_PORT_NUM]);
    DEBUGMSG(ZONE_PDD,(_T("PDD: Set Feature (TEST MODE) OTG PORTSC (0x%x)  PTC=%d\r\n"), (int)*pTsc, portsc.PTC) );

    return TRUE;
#else
    return FALSE;
#endif
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IOControl
//
//  This is IOControl called for UfnPdd request from MDD
//
//  Parameters:
//
//     [IN] pPddContext - pointer to USBFN_PDD
//     [IN] source - IOCTL_SOURCE
//     [IN] code - io request
//     [IN] pInBuffer - pointer to IN buffer from MDD
//     [IN] inSize - pInBuffer size
//     [OUT] pOutBuffer - pointer to OUT buffer to MDD
//     [IN] outSize - size of out buffer available
//     [OUT] pOutSize - actual out buffer being return to MDD
//
//  Return:
//      ERROR_SUCCESS : success, otherwise - failure
//-----------------------------------------------------------------------------
DWORD WINAPI UfnPdd_IOControl(
                              VOID *pPddContext, IOCTL_SOURCE source,
                              DWORD code, UCHAR *pInBuffer, DWORD inSize,
                              UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize
                             )
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    USBFN_PDD *pPdd = pPddContext;
    UFN_PDD_INFO info;
    CE_BUS_POWER_STATE *pBusPowerState;
    CEDEVICE_POWER_STATE devicePowerState;
    PVOID pDestMarshalled = NULL;      // WinCE 6.0 pointer marshalling.

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pOutSize);

    LOCK();
    switch (code)
    {
    case IOCTL_UFN_GET_PDD_INFO:
        {
            if (source != BUS_IOCTL)
                break;

            if (pOutBuffer == NULL || outSize < sizeof(UFN_PDD_INFO))
                break;

            info.InterfaceType = Internal;
            info.BusNumber = 0;
            info.dwAlignment = sizeof(DWORD);
            if (!memcpy(pOutBuffer, &info, sizeof(UFN_PDD_INFO)))
                break;

            rc = ERROR_SUCCESS;
        }
        break;

    case IOCTL_BUS_GET_POWER_STATE:
        {
            DEBUGMSG(ZONE_POWER, (TEXT("IOCTL_BUS_GET_POWER_STATE ")
                                   TEXT("called\r\n")));
            if (source != MDD_IOCTL)
                break;

            if (pInBuffer == NULL || inSize < sizeof(CE_BUS_POWER_STATE))
                break;

            pBusPowerState = (CE_BUS_POWER_STATE*)pInBuffer;

            // WinCE 6.0 requires embedded pointers to be explicitly
            // accessed-checked and marshalled.
            if (CeOpenCallerBuffer(&pDestMarshalled,
                                   pBusPowerState->lpceDevicePowerState,
                                   sizeof(CEDEVICE_POWER_STATE),
                                   ARG_O_PTR,
                                   FALSE) == S_OK)
            {
                if (memcpy(pDestMarshalled,
                           &pPdd->m_CurPMPowerState,
                           sizeof(CEDEVICE_POWER_STATE)))
                {
                    rc = ERROR_SUCCESS;
                }

                if (CeCloseCallerBuffer(pDestMarshalled,
                                        pBusPowerState->lpceDevicePowerState,
                                        sizeof(CEDEVICE_POWER_STATE),
                                        ARG_O_PTR) != S_OK)
                {
                    DEBUGMSG(ZONE_PDD, (_T("IOCTL_BUS_GET_POWER_STATE: ")
                                        _T("CeCloseCallerBuffer() ")
                                        _T("failed\r\n")));
                }
            }
            else
            {
                DEBUGMSG(ZONE_PDD, (_T("IOCTL_BUS_GET_POWER_STATE: ")
                                    _T("CeOpenCallerBuffer() ")
                                    _T("failed\r\n")));
            }
        }
        break;

    case IOCTL_BUS_SET_POWER_STATE:
        {
            DEBUGMSG(ZONE_POWER, (TEXT("IOCTL_BUS_SET_POWER_STATE ")
                                   TEXT("called\r\n")));
            if (source != MDD_IOCTL)
                break;

            if (pInBuffer == NULL || inSize < sizeof(CE_BUS_POWER_STATE))
                break;

            pBusPowerState = (CE_BUS_POWER_STATE*)pInBuffer;

            // WinCE 6.0 requires embedded pointers to be explicitly
            // accessed-checked and marshalled.
            if (CeOpenCallerBuffer(&pDestMarshalled,
                                   pBusPowerState->lpceDevicePowerState,
                                   sizeof(CEDEVICE_POWER_STATE),
                                   ARG_I_PTR,
                                   FALSE) == S_OK)
            {
                if (memcpy(&devicePowerState,
                           pDestMarshalled,
                           sizeof(CEDEVICE_POWER_STATE)))
                {
                    SetPowerState(pPdd , devicePowerState ) ;
                    rc = ERROR_SUCCESS;
                }

                if (CeCloseCallerBuffer(pDestMarshalled,
                                        pBusPowerState->lpceDevicePowerState,
                                        sizeof(CEDEVICE_POWER_STATE),
                                        ARG_I_PTR) != S_OK)
                {
                    DEBUGMSG(ZONE_PDD, (_T("IOCTL_BUS_SET_POWER_STATE: ")
                                        _T("CeCloseCallerBuffer() ")
                                        _T("failed\r\n")));
                }
            }
            else
            {
                DEBUGMSG(ZONE_PDD, (_T("IOCTL_BUS_SET_POWER_STATE: ")
                                    _T("CeOpenCallerBuffer() ")
                                    _T("failed\r\n")));
            }
        }
        break;

    default:
        break;
    }

    UNLOCK();
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Deinit
//
//  This is called by MDD during PDD is deinitialized
//
//  Parameters:
//
//      pPddContext - pointer to USBFN_PDD
//
//  Return:
//      ERROR_SUCCESS
//
//-------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_Deinit(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;

    // Stop interrupt thread
    if (pPdd->hIntrThread != NULL)
    {
        pPdd->exitIntrThread = TRUE;
        DEBUGMSG(ZONE_PDD, (L"Deinit\r\n"));
        if(pPdd->hFunctionEvent)
            SetEvent(pPdd->hFunctionEvent);
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

    // Close function event handler
    if (pPdd->hFunctionEvent != NULL)
    {
        CloseHandle(pPdd->hFunctionEvent);
        pPdd->hFunctionEvent = NULL;
    }

    // Close transfer event handler
    if (pPdd->hTransferEvent != NULL)
    {
        CloseHandle(pPdd->hTransferEvent);
        pPdd->hTransferEvent = NULL;
    }

        // Close transceiver event handler
    if (pPdd->hXcvrEvent != NULL)
    {
        CloseHandle(pPdd->hXcvrEvent);
        pPdd->hXcvrEvent = NULL;
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
        // KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR,
        //                 &pPdd->sysIntr,
        //                 sizeof(pPdd->sysIntr),
        //                 NULL, 0, NULL
        //                );
        pPdd->sysIntr = 0;
    }

    // Delete critical section
    DeleteCriticalSection(&pPdd->epCS);

    DeleteCriticalSection(&g_csRegister);

    BSPUSBClockDeleteFileMapping();

    if ( pPdd->qhbuffer )
    {
        FreePhysMem(pPdd->qhbuffer);
        pPdd->qhbuffer = NULL;
    }

    HalFreeCommonBuffer(&Adapter1,
                        MAX_SIZE_PER_BP,
                        phyAddr_TD1,
                        Buffer_Td1,
                        FALSE);

    HalFreeCommonBuffer(&Adapter2,
                        MAX_SIZE_PER_BP,
                        phyAddr_TD2,
                        Buffer_Td2,
                        FALSE);

    // Free PDD context
    LocalFree(pPdd);

    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_DeregisterDevice
//
//  This function is called by MDD to move device to pre-registred state.
//  On Freescale iMx31 we do nothing.
//
//  Parameter:
//    pPddContext - pointer to USBFN_PDD
//
//  Return:
//    ERROR_SUCCESS
//
//-----------------------------------------------------------------------------
DWORD WINAPI UfnPdd_DeregisterDevice(VOID *pPddContext)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pPddContext);

    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Stop
//
//  This function is called before UfnPdd_DeregisterDevice. It should de-attach
//  device to USB bus (but we don't want disable interrupts because...)
//
//  Parameters:
//    pPddContext - pointer to USBFN_PDD
//
//  Return:
//     ERROR_SUCCESS
//
//------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_Stop(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    CEDEVICE_POWER_STATE prevPowerState ;

    LOCK();
    DEBUGMSG(ZONE_PDD, (_T("UfnPdd_Stop\r\n")));
    prevPowerState = pPdd->m_CurSelfPowerState ;
    if (prevPowerState == D3 || prevPowerState== D4)
    {
        pPdd->m_CurSelfPowerState = D2;
        UpdateDevicePower(pPdd);
    }

    BSPUSBClockDisable(FALSE);

     OUTREG32(&pUSBDRegs->OTG.USBINTR, 0);

    // Reset all interrupts
    {
        USB_USBSTS_T temp;
        DWORD       *t = (DWORD*)&temp;
        *t=0;
        temp.UI=1;   temp.UEI=1; temp.PCI=1; temp.FRI=1;
        temp.SEI=1; temp.AAI=1; temp.URI=1; temp.SRI=1;
        temp.SLI=1; temp.ULPII=1;
        OUTREG32(&pUSBDRegs->OTG.USBSTS, *t);
    }

    // Deattach device
    {
        USB_USBCMD_T cmd;
        DWORD * temp=(DWORD * )&cmd;
        *temp=0; cmd.RS=1;
        CLRREG32(&pUSBDRegs->OTG.USBCMD,*temp);
    }

    pPdd->m_CurSelfPowerState = prevPowerState;
    UpdateDevicePower(pPdd);

    UNLOCK();
    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_DeinitEndpoint
//
//  This function is called when pipe to endpoint is closed. For Freescale
//  iMx31 we will stop points in UfnPdd_DeregisterDevice.
//
//  Parameters:
//
//      pPddContext - Pointer to USBFN_PDD
//      endPoint - endpoint to be deinit
//
//-----------------------------------------------------------------------------
DWORD WINAPI UfnPdd_DeinitEndpoint(VOID *pPddContext, DWORD endPoint)
{
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    DEBUGMSG(ZONE_PDD, (L"+UfnPdd_DeinitEndpoint: %d\r\n", endPoint));

    LOCK();
    // Select EP
    epNum = USBD_EP_NUM & endPoint;

    // Flush EP
    {
        USB_ENDPTFLUSH_T flush;
        USB_ENDPTCTRL_T ctrl;
        DWORD * temp=(DWORD * )&flush;
        *temp=0;
        if (pPdd->ep[epNum]->dirRx)
            flush.FERB|=(1<<epNum);
        else
            flush.FETB|=(1<<epNum);

        CLRREG32(&pUSBDRegs->OTG.ENDPTFLUSH,*temp);
        temp=(DWORD * )&ctrl;
        *temp=0;
        if (pPdd->ep[epNum]->dirRx)
            ctrl.RXE=1;
        else
            ctrl.TXE=1;

        CLRREG32(&pUSBDRegs->OTG.ENDPTCTRL[epNum-1],*temp);
    }

    UNLOCK();
    // Done
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_InitEndpoint
//
//  This function is called when pipe to endpoint is created.
//
//  Parameters:
//    pContext - Pointer to USBFN_PDD
//    endPoint - Endpoint to be initialized
//    speed - Not used
//    pEPDesc - Pointer to endpoint descriptor
//    pReserved - Not used
//    configValue - Not used
//    interfaceNumber - Not used
//    alternateSetting - Not used
//
//  Return:
//    ERROR_SUCCESS
//
//-------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_InitEndpoint(
                                 VOID *pContext, DWORD endPoint,
                                 UFN_BUS_SPEED speed,
                                 USB_ENDPOINT_DESCRIPTOR *pEPDesc,
                                 VOID *pReserved, UCHAR configValue,
                                 UCHAR interfaceNumber,
                                 UCHAR alternateSetting
                                )
{

    USBFN_PDD *pPdd = pContext;
    USB_ENDPTCTRL_T edptctrl;
    DWORD epNum, *temp=(DWORD *)&edptctrl;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(speed);
    UNREFERENCED_PARAMETER(pReserved);
    UNREFERENCED_PARAMETER(configValue);
    UNREFERENCED_PARAMETER(interfaceNumber);
    UNREFERENCED_PARAMETER(alternateSetting);

    DEBUGMSG(ZONE_PDD, (_T("UfnPdd_InitEndpoint: %d Attrib(%d)\r\n"), endPoint,
                pEPDesc->bmAttributes));

    LOCK();

    // Select EP
    epNum = USBD_EP_NUM & endPoint;
    *temp=0;
    if (epNum>0)
    {
        // Init the end point to make sure it is correct state
        pPdd->ep[epNum]->pTransfer = NULL;
        pPdd->ep[epNum]->pMappedBufPtr = 0;
        pPdd->ep[epNum]->dwNextPageIdx = 0;
        pPdd->ep[epNum]->dwNumPages = 0;
        pPdd->ep[epNum]->bPagesLocked = FALSE;

        memset(&pPdd->qhbuffer->qh[epNum*2], 0, sizeof(USBD_dQH_T));
        memset(&pPdd->qhbuffer->qh[(epNum*2)+1], 0, sizeof(USBD_dQH_T));
        memset(&pPdd->qhbuffer->td[epNum*2], 0, sizeof(USBD_dTD_R_T));
        memset(&pPdd->qhbuffer->td[(epNum*2)+1], 0, sizeof(USBD_dTD_R_T));
        pPdd->qhbuffer->bPrimed[epNum] = FALSE;

        if (!pPdd->ep[epNum]->dirRx)
        {
            edptctrl.TXT=pEPDesc->bmAttributes&USB_ENDPOINT_TYPE_MASK;

            edptctrl.TXR=1;
            edptctrl.TXE=1;
        }
        else
        {
            edptctrl.RXT=pEPDesc->bmAttributes&USB_ENDPOINT_TYPE_MASK;
            edptctrl.RXR=1;
            edptctrl.RXE=1;
        }
        OUTREG32(&pPdd->pUSBDRegs->OTG.ENDPTCTRL[epNum-1], *temp);
        DEBUGMSG(ZONE_PDD, (L"Init Endpoint: %x, ctrl=%x\r\n",endPoint,
                            INREG32(&pPdd->pUSBDRegs->OTG.ENDPTCTRL[epNum-1])));
    }

    UNLOCK();

    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_SetAddress
//
//  This function is called by MDD when configuration process assigned address
//  to device.
//
//  Parameters:
//    pPddContext - Pointer to USBFN_PDD
//    address - Device address to set
//
//  Return:
//    ERROR_SUCCESS
//
//-----------------------------------------------------------------------------
DWORD WINAPI UfnPdd_SetAddress(VOID *pPddContext, UCHAR address)
{

    USBFN_PDD *pPdd =(USBFN_PDD *) pPddContext;

    LOCK();
    pPdd->addr=address;
    // This seems a so ugly trick
    pPdd->addr|=0x80;

    UNLOCK();
    return ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_Start
//
//  This function is called after UfnPdd_RegisterDevice. It should attach
//  device to USB bus.
//
//  Parameters:
//    pPddContext - Pointer to USBFN_PDD
//
//  Return:
//    ERROR_SUCCESS
//
//-----------------------------------------------------------------------------
DWORD WINAPI UfnPdd_Start(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS*pUSBDRegs = pPdd->pUSBDRegs;
    int i;


    CEDEVICE_POWER_STATE prevPowerState = pPdd->m_CurSelfPowerState ;

    LOCK();

    DEBUGMSG(ZONE_PDD, (TEXT("+UfnPdd_Start\r\n")));
    if (prevPowerState == D3 || prevPowerState== D4)
    {
        pPdd->m_CurSelfPowerState = D2;
        UpdateDevicePower(pPdd);
    }

   BSPUSBClockDisable(FALSE);

    OUTREG32(&pUSBDRegs->OTG.T_158H.ENDPOINTLISTADDR,pPdd->qhbuf_phy);

    // Enable interrupts
#ifdef USBCV_FIX
    //disable SLI interrupt
    OUTREG32(&pUSBDRegs->OTG.USBINTR, USBD_IRQ_MASK);
    SETREG32(&pUSBDRegs->OTG.OTGSC, USBD_OTGSC_BSVIE); //enable BSVIE
    RETAILMSG(DEBUG_LOG_USBCV, (L"otgsc %x\r\n", INREG32(&pUSBDRegs->OTG.OTGSC)));
#else
    OUTREG32(&pUSBDRegs->OTG.USBINTR, USBD_IRQ_MASK);
#endif

    // initial endpoints
    for (i=0;i<15;i++)
        OUTREG32(&pUSBDRegs->OTG.ENDPTCTRL[i], 0x400040);

    OUTREG32(&pUSBDRegs->OTG.T_154H.USBADR, 0);

    // Set the run bit
    SETREG32(&pUSBDRegs->OTG.USBCMD, 1);

    UNLOCK();

    InitQh0(pPdd,0, NULL);

    // Set fake device change flag which on first interrupt force
    // device state change handler even if it isn't indicated by hardware
    pPdd->m_CurSelfPowerState = prevPowerState;
    UpdateDevicePower(pPdd);

    //Shifting the transfer to the transceiver when the cable is not connected
    if ((pPdd->bInUSBFN == FALSE) && (g_isInit == TRUE))
    {
        g_isInit = FALSE;
        SetEvent(pPdd->hTransferEvent);
    }
    DEBUGMSG(ZONE_PDD, (TEXT("-UfnPdd_Start\r\n")));

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
//  UfnPdd_InitEndpoint.
//
//
//  Parameters:
//    pContext - Pointer to USBFN_PDD
//    pHighSpeedDeviceDesc - Pointer to High Speed USB Device descriptor
//    pHighSpeedConfig - Pointer to USB Configuration
//    pFullSpeedDeviceDesc - Pointer to Full Speed USB Device descriptor
//    pFullSpeedConfig - Pointer to USB Configuration
//    pFullSpeedConfigDesc - Pointer to USB Configuration descriptor
//
//  Return:
//    ERROR_SUCCESS
//
//-------------------------------------------------------------------------------

DWORD WINAPI UfnPdd_RegisterDevice(
                                   VOID *pPddContext,
                                   const USB_DEVICE_DESCRIPTOR *pHighSpeedDeviceDesc,
                                   const UFN_CONFIGURATION *pHighSpeedConfig,
                                   const USB_CONFIGURATION_DESCRIPTOR *pHighSpeedConfigDesc,
                                   const USB_DEVICE_DESCRIPTOR *pFullSpeedDeviceDesc,
                                   const UFN_CONFIGURATION *pFullSpeedConfig,
                                   const USB_CONFIGURATION_DESCRIPTOR *pFullSpeedConfigDesc,
                                   const UFN_STRING_SET *pStringSets,
                                   DWORD stringSets
                                  )
{
    USBFN_PDD *pPdd = pPddContext;
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT *pEP;
    DWORD offset, ep;
    DWORD ifc, epx;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(pHighSpeedConfigDesc);
    UNREFERENCED_PARAMETER(pFullSpeedConfigDesc);
    UNREFERENCED_PARAMETER(pStringSets);
    UNREFERENCED_PARAMETER(stringSets);

    LOCK();

    // Remember self powered flag
    pPdd->selfPowered = (pFullSpeedConfig->Descriptor.bmAttributes & 0x20) != 0;
    pPdd->selfPoweredh = (pHighSpeedConfig->Descriptor.bmAttributes & 0x20) != 0;

    // Configure Full speed EPs
    offset = 8;
    // Configure EP0
    pPdd->epf[0].maxPacketSize = pFullSpeedDeviceDesc->bMaxPacketSize0;
    offset += pFullSpeedDeviceDesc->bMaxPacketSize0;

    for (ifc = 0; ifc < pFullSpeedConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pFullSpeedConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
        {
            pEP = &pIFC->pEndpoints[epx];
            // Get EP address
            ep = pEP->Descriptor.bEndpointAddress & 0x0F;
            // If it is Tx EP skip it
            if ((pEP->Descriptor.bEndpointAddress & 0x80) != 0)
            {
                pPdd->epf[ep].dirRx = FALSE;
            }
            else
            {
                pPdd->epf[ep].dirRx = TRUE;
            }

            // Save max packet size & direction
            pPdd->epf[ep].maxPacketSize = pEP->Descriptor.wMaxPacketSize;

            // Update offset
            offset += pEP->Descriptor.wMaxPacketSize;
        }
    }

    // Final Hardcode the Endpoint 0
    pPdd->epf[USBD_EP_COUNT-1].maxPacketSize = pPdd->epf[0].maxPacketSize;
    pPdd->epf[USBD_EP_COUNT-1].dirRx = FALSE;
    pPdd->epf[0].dirRx = TRUE;

    // Configure High speed EPs

    // Configure EP0
    pPdd->eph[0].maxPacketSize = pHighSpeedDeviceDesc->bMaxPacketSize0;
    pPdd->eph[USBD_EP_COUNT-1].maxPacketSize =
        pHighSpeedDeviceDesc->bMaxPacketSize0;
    offset += pHighSpeedDeviceDesc->bMaxPacketSize0;

    for (ifc = 0; ifc < pHighSpeedConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        pIFC = &pHighSpeedConfig->pInterfaces[ifc];
        for (epx = 0; epx < pIFC->Descriptor.bNumEndpoints; epx++)
        {
            pEP = &pIFC->pEndpoints[epx];
            // Get EP address
            ep = pEP->Descriptor.bEndpointAddress & 0x0F;
            // If it is Tx EP skip it
            if ((pEP->Descriptor.bEndpointAddress & 0x80) != 0)
            {
                pPdd->eph[ep].dirRx = FALSE;
            }
            else
            {
                pPdd->eph[ep].dirRx = TRUE;
            }

            // Save max packet size & direction
            pPdd->eph[ep].maxPacketSize = pEP->Descriptor.wMaxPacketSize;

            // Update offset
            offset += pEP->Descriptor.wMaxPacketSize;
        }
    }

    // Final Hardcode the Endpoint 0
    pPdd->eph[USBD_EP_COUNT-1].maxPacketSize = pPdd->eph[0].maxPacketSize;
    pPdd->eph[USBD_EP_COUNT-1].dirRx = FALSE;
    pPdd->eph[0].dirRx = TRUE;

    UNLOCK();

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
//------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_IsEndpointSupportable(
    VOID *pPddContext, DWORD endPoint, UFN_BUS_SPEED speed,
    USB_ENDPOINT_DESCRIPTOR *pEPDesc, UCHAR configurationValue,
    UCHAR interfaceNumber, UCHAR alternateSetting
    )
{
    UNREFERENCED_PARAMETER(pPddContext);
    UNREFERENCED_PARAMETER(speed);
    UNREFERENCED_PARAMETER(configurationValue);
    UNREFERENCED_PARAMETER(interfaceNumber);
    UNREFERENCED_PARAMETER(alternateSetting);

    // Update maximal packet size for EP0
    if (endPoint == 0)
    {
        DEBUGCHK(pEPDesc->bmAttributes == USB_ENDPOINT_TYPE_CONTROL);
        pEPDesc->wMaxPacketSize = USB_FULL_HIGH_SPEED_CONTROL_MAX_PACKET_SIZE;
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
//  For iMX31 we should check if total descriptor size is smaller
//  than MAX_SIZE_PER_TD bytes and round EP sizes. Unfortunately we don't
//  get information about EP0 max packet size. So we will assume maximal
//  64 byte size.
//
// -------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_IsConfigurationSupportable(
    VOID *pPddContext, UFN_BUS_SPEED speed, UFN_CONFIGURATION *pConfig
    )
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT *pEP;
    WORD ifc, epx, count;
    WORD offset, size;

    UNREFERENCED_PARAMETER(pPddContext);

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
            size = 1 << (WORD)(Log2(size));
            // Is it ISO end point?
            if ((pEP->Descriptor.bmAttributes & 0x03) != 0x01)
            {
                if (speed == BS_HIGH_SPEED)
                {
                    // Non-ISO, max size is 512 bytes for High-speed
                    if (size > 512) size = 512;
                }
                else
                {
                    // Non-ISO, max size is 64 bytes for Full/Low-speed
                if (size > 64) size = 64;
                }
            }
            else
            {
                // ISO edpoint, maximal size is 512 bytes
                if (size > 512) size = 512;
            }
            // Update EP size
            pEP->Descriptor.wMaxPacketSize = size;
            // Calculate total buffer size
            offset = (WORD)(offset + size);
        }
        // Add number of end points to total count
        count = (WORD)(count + pIFC->Descriptor.bNumEndpoints);
        DEBUGMSG(ZONE_PDD, (TEXT("..count %d\r\n"),count));
    }

    // Can we support this configuration?
    if (count < USBD_EP_COUNT && offset <= MAX_SIZE_PER_TD)
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
//  Parameters:
//      szActiveKey : Pointer to the registry path
//      pMddContext : Pointer to the MDD Context
//      pMddIfc : Pointer to UFN_MDD_INTERFACE_INFO
//      pPddIfc : Pointer to UFN_PDD_INTERFACE_INFO
//
//  Return:
//
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_Init(
                         LPCTSTR szActiveKey,
                         VOID *pMddContext,
                         UFN_MDD_INTERFACE_INFO *pMddIfc,
                         UFN_PDD_INTERFACE_INFO *pPddIfc
                         )
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    USBFN_PDD *pPdd;
    CSP_USB_REGS *pUSBDRegs;
    //DEVICE_LOCATION devLoc;
    PHYSICAL_ADDRESS pa;
    DWORD len;
    DWORD *pSREV=NULL;
    //DWORD ep;

    // Allocate PDD object
    pPdd = LocalAlloc(LPTR, sizeof(USBFN_PDD));
    if (pPdd == NULL)
    {
        DEBUGMSG(ZONE_PDD, (TEXT("UfnPdd_Init pPdd allocate failed\r\n")));
        goto clean;
    }

    memset(pPdd, 0, sizeof(USBFN_PDD));

    BSPUSBClockCreateFileMapping();

    InitializeCriticalSection(&g_csRegister);

    // Initialize critical section
    InitializeCriticalSection(&pPdd->epCS);
    pPdd->devState = 0;
    pPdd->bInUSBFN = TRUE;
    pPdd->bResume = FALSE;
    pPdd->bEnterTestMode = FALSE;
    pPdd->bUSBCoreClk = TRUE;
    //Map SREV register - Used for Tape Out identification, for removal of PANIC mode changes in TO2
    pa.QuadPart = (CSP_BASE_REG_PA_IIM + 0x24);
    len = 0x04; // Four BYTES
    pSREV = MmMapIoSpace(pa, len, FALSE);
    if(pSREV ==NULL){
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: SREV register mapping failed\r\n"));
        goto clean;
    }

    if(((*pSREV)&0xf0) == UFN_TO2)
    {
        pPdd->bIsMX31TO2 = TRUE;
    }
    else
    {
        pPdd->bIsMX31TO2 = FALSE;
    }
    RETAILMSG(FALSE,(TEXT("pPdd->bIsMX31TO2  = %d"),pPdd->bIsMX31TO2));

    if (pSREV)
    {
       MmUnmapIoSpace((VOID*)pSREV, 0x04);
       pSREV = NULL;
    }

    if(pPdd->bIsMX31TO2 )
       pPdd->bUSBPanicMode = TRUE;


    // Read device parameters
    if (GetDeviceRegistryParams(szActiveKey, pPdd, dimof(g_deviceRegParams),
                                g_deviceRegParams) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: "
                              L"Failed read registry parameters\r\n"));
        goto clean;
    }

    // Set PM to Default
    pPdd->m_CurPMPowerState = PwrDeviceUnspecified ;
    pPdd->m_CurSelfPowerState = D0;
    pPdd->m_CurActualPowerState = D0 ;
    pPdd->hParentBus = CreateBusAccessHandle(szActiveKey);
    if (pPdd->hParentBus == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Failed open bus driver\r\n"));
        goto clean;
    }

    // Set hardware to standby mode
    pPdd->m_CurSelfPowerState = D2;
    UpdateDevicePower(pPdd);

    // Map the USB registers
    pa.QuadPart = pPdd->memBase;
    pUSBDRegs = MmMapIoSpace(pa, pPdd->memLen, FALSE);
    if (pUSBDRegs == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Controller registers "
                              L"mapping failed\r\n"));
        rc=ERROR_NOT_ENOUGH_MEMORY;
        goto clean;
    }
    pPdd->pUSBDRegs = pUSBDRegs;

    {
        int i;
        for (i=0;i<USBD_EP_COUNT;i++)
        {
            pPdd->ep[i]=&pPdd->epf[i];
        }
    }
    if (HardwareInitialize(pPdd->pUSBDRegs)==FALSE)
    {
        goto clean;
    }

    OUTREG32(&pUSBDRegs->OTG.USBINTR, 0);


    // Reset all interrupts
    {
        USB_USBSTS_T temp;
        DWORD       *t = (DWORD*)&temp;
        *t=0;
        temp.UI=1;   temp.UEI=1; temp.PCI=1; temp.FRI=1;
        temp.SEI=1; temp.AAI=1; temp.URI=1; temp.SRI=1;
        temp.SLI=1; temp.ULPII=1;
        OUTREG32(&pUSBDRegs->OTG.USBSTS, *t);
    }

    // Should set WINCEOEM=1
    DEBUGMSG(ZONE_PDD, (TEXT("Sizeof qhbuffer = 0x%x\r\n"), sizeof(USBFN_QH_BUF_T)));
    pPdd->qhbuffer = (USBFN_QH_BUF_T*)AllocPhysMem(sizeof(USBFN_QH_BUF_T),
        PAGE_READWRITE|PAGE_NOCACHE,
        0,        // is_this_correct                   // 4k alignment
        0,    // Reserved
        &pPdd->qhbuf_phy);
    if (pPdd->qhbuffer==NULL)
    {
        rc=ERROR_NOT_ENOUGH_MEMORY;
        goto clean;
    }
    memset(pPdd->qhbuffer, 0, sizeof(USBFN_QH_BUF_T));

    //RETAILMSG(1,(L"qhbuf_phy=%x, qhbuffer=%x\r\n", pPdd->qhbuf_phy, pPdd->qhbuffer));
    pPdd->m_CurSelfPowerState = D4; // Assume Detached First.
    UpdateDevicePower(pPdd);
#ifdef USB_ELEGANT_INTR
    {
        INT32 aIrqs[3];
        aIrqs[0] = -1;
        aIrqs[1] = OAL_INTR_TRANSLATE;
        aIrqs[2] = IRQ_USB_OTG;
        KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, aIrqs, sizeof(aIrqs), &pPdd->sysIntr, sizeof(pPdd->sysIntr), NULL);
    }
#else
    // Request SYSINTR for interrupts
    pPdd->sysIntr = GetSysIntr();
#endif

    // Create interrupt event
    pPdd->hIntrEvent = CreateEvent(0, FALSE, FALSE, NULL);
    if (pPdd->hIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Error creating interrupt "
                              L"event\r\n"));
        goto clean;
    }

    // Initialize interrupt
    if (!InterruptInitialize(pPdd->sysIntr, pPdd->hIntrEvent, NULL, 0))
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Interrupt initialization "
                              L"failed\r\n"));
        goto clean;
    }

    DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFNPDD SysIntr:0x%x, Irq:0x%x\r\n"),
                               pPdd->sysIntr, pPdd->irq));

    if (pPdd->IsOTGSupport)
    {
        DEBUGMSG(ZONE_INTERRUPTS, (TEXT("Disable Interrupt and let the Transceiver ")
                     TEXT("handle that\r\n")));
        InterruptDisable(pPdd->sysIntr);
        g_isInit = TRUE;
    }
    else
    {
        // Enable Kernel WakeUp
        /*KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &pPdd->sysIntr,
                        sizeof(pPdd->sysIntr), NULL, 0, NULL);*/
    }

    // Set PDD interface
    pPddIfc->dwVersion = UFN_PDD_INTERFACE_VERSION;
    pPddIfc->dwCapabilities = UFN_PDD_CAPS_SUPPORTS_FULL_SPEED |
                              UFN_PDD_CAPS_SUPPORTS_HIGH_SPEED | UFN_PDD_CAPS_SUPPORTS_MULTIPLE_CONFIGURATIONS;
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

    // Run interrupt thread
    pPdd->exitIntrThread = FALSE;
    pPdd->hIntrThread = CreateThread(NULL, 0, InterruptThread, pPdd, 0, NULL);
    if (pPdd->hIntrThread == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Interrupt thread "
                              L"creation failed\r\n"));
        goto clean;
    }
    CeSetThreadPriority( pPdd->hIntrThread, pPdd->priority256 ) ;
    // Allocate uncached memory to handle SDMA Cache allignment issue
    memset(&Adapter1, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter1.InterfaceType = Internal;
    Adapter1.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    // allocate DMA compatible memory

    Buffer_Td1 = HalAllocateCommonBuffer(
                                         &Adapter1,
                                         MAX_SIZE_PER_BP,
                                         &phyAddr_TD1,
                                         FALSE);

    if(Buffer_Td1 == NULL)
    {
        DEBUGMSG(ZONE_PDD, (TEXT("Allocate DMA memory for Buffer_Td1 failed\r\n")));
    }

    memset(&Adapter2, 0, sizeof(DMA_ADAPTER_OBJECT));
    Adapter2.InterfaceType = Internal;
    Adapter2.ObjectSize = sizeof(DMA_ADAPTER_OBJECT);

    Buffer_Td2 = HalAllocateCommonBuffer(
                                         &Adapter2,
                                         MAX_SIZE_PER_BP,
                                         &phyAddr_TD2,
                                         FALSE);

    if(Buffer_Td2 == NULL)
    {
        DEBUGMSG(ZONE_PDD, (TEXT("Allocate DMA memory for Buffer_Td2 failed\r\n")));
    }

    // Done
    rc = ERROR_SUCCESS;

clean:
    return rc;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_DllEntry
//
//  DLL Entry for PDD driver
//
//  Parameters:
//      hDllHandle - Handle to the DLL
//      reason - Reason to call this entry
//      pReserved - Pointer to reserved data
//
//  Return:
//
//      ERROR_SUCCESS
//
//------------------------------------------------------------------------------
extern BOOL UfnPdd_DllEntry(
   HANDLE hDllHandle, DWORD reason, VOID *pReserved)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(hDllHandle);
    UNREFERENCED_PARAMETER(reason);
    UNREFERENCED_PARAMETER(pReserved);

    return TRUE;
}

//#pragma optimize( "", on )


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function:  CheckAndHandleUnsupportedRequests
//
//  This function handles unsupported usb requests, work around for h/w limitations.
//
//  Parameter:
//      pPdd - Pointer to USBFN_PDD
//      pUdr - Pointer to PUSB_DEVICE_REQUEST
//
//  Return:
//      TRUE - if request passed is unsupported and handled by here.
//      FALSE - Supported request that should be passed to MDD.
//
//------------------------------------------------------------------------------
BOOL CheckAndHandleUnsupportedRequests(USBFN_PDD *pPddContext, PUSB_DEVICE_REQUEST pUdr)
{
    BOOL bIsUnSupported = FALSE;

    // Process device-to-host requests
    if ( pUdr->bmRequestType ==
         (USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE) )
    {
        // Process device requests
        switch (pUdr->bRequest) {
            //Handle any unsupported request here...
            default:
                break;
        }
    }
    else if ( pUdr->bmRequestType ==
        (USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_STANDARD | USB_REQUEST_FOR_INTERFACE) )
    {
        // Process interface requests
        switch (pUdr->bRequest) {
            case USB_REQUEST_GET_INTERFACE:
                //Only 1 Interface is supported and there is only 1 alternate settings
                //so the device will STALL on this;
                //Refer to USB spec section 9.4.4 GET_INTERFACE - It says the following:
                //This  request will only return the "SELECTED" alternate setting for a specified interface,
                //so it should STALL
                //in our case because we dont support multiple alternate setting and multiple interfaces.
                //So client should not call Getinterface.It can assume the default and shall proceed.
                bIsUnSupported = TRUE;
                break;
            default:
                break;
        }
    }
    else if ( pUdr->bmRequestType ==
        (USB_REQUEST_DEVICE_TO_HOST | USB_REQUEST_STANDARD | USB_REQUEST_FOR_ENDPOINT) )
    {
        // Process interface requests
        switch (pUdr->bRequest) {
            case USB_REQUEST_SYNC_FRAME:
                bIsUnSupported = TRUE;
                break;

            default:
                break;
        }
    }
    // Process host-to-device requests
    else if ( pUdr->bmRequestType ==
        (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_DEVICE) )
    {
        switch (pUdr->bRequest) {
            case USB_REQUEST_SET_FEATURE:
                if(pUdr->wLength != 0 )
                {
                    bIsUnSupported = TRUE;
                }
                #ifdef USBFN_TEST_MODE_SUPPORT
                if (pUdr->wValue == USB_FEATURE_TEST_MODE)
                {
                    int iTestMode;
                    iTestMode = (pUdr->wIndex >> 8) & 0xFF;

                    if (((pUdr->wIndex & 0xFF) == 0) && (iTestMode <= USB_TEST_MODE_MAX))
                    {
                        pPddContext->bEnterTestMode = TRUE;
                        pPddContext->iTestMode = iTestMode;
                        DEBUGMSG(ZONE_PDD, (TEXT("Test mode requested: %d\r\n"), iTestMode));

                        // Handling the Test mode request
                        SetEvent(pPddContext->hIntrEvent);
                        //MDD does not support Test mode.It will be handled in the IST, hence
                        //we send the status handshake and are returning from here as
                        //unsupported request.
                        UfnPdd_SendControlStatusHandshake(pPddContext, 0);
                        return TRUE;
                     }
                }
                #endif
                bIsUnSupported = TRUE;

                break;
            case USB_REQUEST_SET_DESCRIPTOR:
                bIsUnSupported = TRUE;
                break;
            default:
                break;
        }
    }
    else if ( pUdr->bmRequestType ==
        (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_INTERFACE) )
    {
        switch (pUdr->bRequest) {
            case USB_REQUEST_SET_INTERFACE:
                //Only 1 Interface is supported and there is only 1 alternate settings
                //so the device will STALL on this;
                //Refer to USB spec section 9.4.10 SET_INTERFACE -It says the following:
                //If a device only supports a default setting for the specified interface,
                //then a STALL may be returned in the Status stage of the request.
                bIsUnSupported = TRUE;
                break;

            case USB_REQUEST_SET_FEATURE:
                bIsUnSupported = TRUE;
            default:
                break;
        }
    }
    else if ( pUdr->bmRequestType ==
        (USB_REQUEST_HOST_TO_DEVICE | USB_REQUEST_STANDARD | USB_REQUEST_FOR_ENDPOINT) )
    {
        switch (pUdr->bRequest) {
            //Handle any unsupported request here...
            default:
                break;
        }
    }

    if (bIsUnSupported)
    {
        UfnPdd_StallEndpoint(pPddContext, 0);
        UfnPdd_SendControlStatusHandshake(pPddContext, 0);
    }

    return bIsUnSupported;
}

