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
// Copyright (C) 2005-2006, Freescale Semiconductor, Inc. All Rights Reserved.
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
#include <windows.h>
#include <nkintr.h>
#include <pm.h>
#include <ceddk.h>
#include <cebus.h>
#include <usbfntypes.h>
#include <usbfn.h>
#include "csp.h"
#include "usbd.h"
#if 1 /*ERIC0907*/
#include <mx27_usbname.h>
#include <mx27_usbcommon.h>
#else
#include <mx31_usbname.h>
#include <mx31_usbcommon.h>
#endif

#include "celog.h"

#define IDLE_TIMEOUT 3000

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
#ifdef DEBUG

#define ZONE_INTERRUPTS         DEBUGZONE(8)
#define ZONE_POWER              DEBUGZONE(9)
#define ZONE_PDD                DEBUGZONE(15)
#define ZONE_WARNING            DEBUGZONE(1)

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

CRITICAL_SECTION g_csRegister;
CRITICAL_SECTION g_csIsoc;
BOOL g_driverStarted;
BOOL g_isStartUp = FALSE;

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
                                  VOID *pPddContext, DWORD endPoint, STransfer *pTransfer
                                  );

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


// bump __tdPtr to point to the next element in the circular list
#define INC_TD_PTR( __ep, __tdPtr )  \
       (__tdPtr) = \
          (   \
             ( (__tdPtr) < &((__ep)->pIsochData->isochtd[USB_ISOCH_TD_ENTRIES-1]) ) ?  \
             (__tdPtr + 1) :  \
             ((__ep)->pIsochData->isochtd)   \
          )

//-------------------------------------------------------------
//
//  Function: ResetIsochEPData
//
//  Resets all the data that in the endpoint that relates to isochronous transfers
//
//  Parameters:
//       pEndpoint - Pointer to the USBFN_EP of the endpoint to reset
//
//--------------------------------------------------------------
static void ResetIsochEPData( USBFN_EP *pEndpoint )
{
    EnterCriticalSection(&g_csIsoc);
    pEndpoint->pIsochData = NULL;
    pEndpoint->pFirstUsedTd = NULL;
    pEndpoint->pFirstFreeTd = NULL;
    pEndpoint->dwNumTdsUsed = 0;
    LeaveCriticalSection(&g_csIsoc);
}

//-------------------------------------------------------------
//
//  Function: GetEpIsochDataIdx
//
//  Resets returns the index of the isoch data slot used by the given endpoint
//
//  Parameters:
//       pPdd - pointer to the PDD data structure
//       pEndpoint - Pointer to the USBFN_EP of the endpoint to reset
//
// Returns:
//       index of block (max is USB_MAX_ISOCH_ENDPOINTS)
//
//--------------------------------------------------------------
static DWORD GetEpIsochDataIdx( USBFN_PDD *pPdd,  USBFN_EP *pEndpoint )
{
    BYTE *addr, *addrBase;
    DWORD retVal;

    EnterCriticalSection(&g_csIsoc);

    addr = (BYTE *)(pEndpoint->pIsochData);
    addrBase = (BYTE *)(pPdd->qhbuffer->isoch_ep_buffers);
    retVal = (addr - addrBase) / sizeof(USBFN_ISOCH_EP_DATA);

    LeaveCriticalSection(&g_csIsoc);

    return retVal;
}

//-------------------------------------------------------------
//
//  Function: GetFreeIsochDataIdx
//
//  Resets returns the index of the isoch data slot used by the given endpoint
//
//  Parameters:
//       pPdd - pointer to the PDD data structure
//       pEndpoint - Pointer to the USBFN_EP of the endpoint to reset
//
//--------------------------------------------------------------
static DWORD GetFreeIsochDataIdx( USBFN_PDD *pPdd )
{
    DWORD rv = 0;
    EnterCriticalSection(&g_csIsoc);
    while( rv < USB_MAX_ISOCH_ENDPOINTS )
    {
        ++rv;
        if( pPdd->bIsochEPBusy[rv] == 0 ) // this one is free
        {
            break;
        }
    }
    LeaveCriticalSection(&g_csIsoc);
    return rv;
}

//-------------------------------------------------------------
//
//  Function: InitIsochEPLinkList
//
//  resets the points to the first and last element in the isochronous linklist endpoint.
//
//  Parameters:
//       pEndpoint - Pointer to the USBFN_EP of the endpoint to reset
//
//--------------------------------------------------------------
static void InitIsochEPLinkList( USBFN_EP *pEndpoint )
{
    EnterCriticalSection(&g_csIsoc);
    pEndpoint->pFirstUsedTd = pEndpoint->pIsochData->isochtd;
    pEndpoint->pFirstFreeTd = pEndpoint->pFirstUsedTd;
    pEndpoint->dwNumTdsUsed = 0;
    LeaveCriticalSection(&g_csIsoc);
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

    while (1)
    {

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
        *temp=INREG32(&pPdd->pUSBDRegs->OTG.USBCMD);

        // check whether a new setup was received while we were retrieving the setup,
        // and retrieve again if so.
        if (cmd.SUTW)
        {
            break;
        }
    }
    *temp=0; cmd.SUTW=1;
    CLRREG32(&pPdd->pUSBDRegs->OTG.USBCMD, *temp);
    //

    // doc claims ENDPTCOMPLETE is set both for Setup and OUT transactions, so it can be normal
    // to have to clear complete now.
    if (INREG32(&(pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE)) & 0x1)
    {
        OUTREG32(&(pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE), 1);
    }

    if ( pPdd->ep[0]->pTransfer )
    {
        DEBUGMSG(ZONE_PDD,
            (TEXT("####### GetSetupPacket: Transfer was in progress on EP0 (EPSTAT 0x%x, status 0x%x)\r\n"),
            INREG32(&(pPdd->pUSBDRegs->OTG.ENDPTSTATUS)),
            pPdd->qhbuffer->qh[0].dtd.status
            ) );

        // that transfer should be aborted.  If EPSTAT was set then it needs also to be flushed
    }

    if ( INREG32(&(pPdd->pUSBDRegs->OTG.ENDPTSTATUS)) & 0x1)
    {
        DEBUGMSG(ZONE_PDD, (TEXT("####### GetSetupPacket: Flushing EP 0 (EPSTATUS 0x%x, QH0:0x%x)\r\n"),
            INREG32(&(pPdd->pUSBDRegs->OTG.ENDPTSTATUS)),
            pPdd->qhbuffer->qh[0].dtd.status) );

        OUTREG32(&pPdd->pUSBDRegs->OTG.ENDPTFLUSH,1);
        /*
        Software note: this operation may take a large amount of time depending on the USB bus
        activity. It is not desirable to have this wait loop within an interrupt service routine.
        *
        * This flush should not normally happen.  Setup only comes while an endpoint is primed, under
        * protocol failure conditions.  Otherwise OUT and IN always complete normally since it is the
        * host which sends SETUP and host which drives the IN/OUT protocol on the control ep.
        */

        while (1)
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
//  This is used to initialise for EP0, where many transactions are set-up transactions.
//  Where actual data is required, this QH is initialised instead through the normal IssueTransfer method.
//
//  Parameters:
//      pPdd - Pointer to USBFN_PDD
//      len - no of bytes to transfer
//      pTransfer - Pointer to STransfer
//
//  Return: NULL
//
//------------------------------------------------------------------
static void InitQh0(USBFN_PDD *pPdd, DWORD len, STransfer *pTransfer)
{
    volatile CSP_USB_REG * pOtgReg=&(pPdd->pUSBDRegs->OTG);
    int retry = 0;

    LOCK();

    pPdd->ep[0]->pTransfer = NULL;
    pPdd->ep[0]->pMappedBufPtr = 0;
    pPdd->ep[0]->dwNextPageIdx = 0;
    pPdd->ep[0]->dwNumPages = 0;
    pPdd->ep[0]->bPagesLocked = FALSE;
    pPdd->ep[0]->bIsochronous = FALSE;
    ResetIsochEPData( pPdd->ep[0] );

    // EP0 is bidirectional, and so it uses two slots in the list.
    pPdd->ep[USBD_EP_COUNT-1]->pTransfer = NULL;
    pPdd->ep[USBD_EP_COUNT-1]->pMappedBufPtr = 0;
    pPdd->ep[USBD_EP_COUNT-1]->dwNextPageIdx = 0;
    pPdd->ep[USBD_EP_COUNT-1]->dwNumPages = 0;
    pPdd->ep[USBD_EP_COUNT-1]->bPagesLocked = FALSE;
    pPdd->ep[USBD_EP_COUNT-1]->bIsochronous = FALSE;
    ResetIsochEPData( pPdd->ep[USBD_EP_COUNT-1] );

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

        DEBUGMSG(ZONE_WARNING,(_T("Setup not processed (SETUPSTAT=0x%x)??? %x, %x\r\n"),
                                _T("%x, %x\r\n"),
                                INREG32(&pOtgReg->ENDPTSETUPSTAT),
                                pPdd->qhbuffer->qh[0].SB0,
                                pPdd->qhbuffer->qh[0].SB1));

        GetSetupPacket(pPdd, data);

        //OUTREG32(&pOtgReg->ENDPTSETUPSTAT, INREG32(&pOtgReg->ENDPTSETUPSTAT));
        DEBUGMSG(ZONE_WARNING,(L"New setup received when priming=%x, %x\r\n", data[0], data[1]));

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

        RETAILMSG(1,(L"PRIME failed on EP0\r\n") );

        if ( INREG32(&pOtgReg->ENDPTSETUPSTAT) & 1 )
        {
            GetSetupPacket(pPdd, data);

            DEBUGMSG(ZONE_PDD,(L"New setup received when priming=%x, %x\r\n", data[0], data[1]));

            DEBUGCHK ( pPdd->ep[0]->bPagesLocked == 0 );

            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SETUP_PACKET, (DWORD)data);

            DEBUGMSG(ZONE_PDD,(L"Status for td0=%x, tb=%x\r\n",
                pPdd->qhbuffer->qh[0].dtd.status,pPdd->qhbuffer->qh[0].dtd.tb));
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
        DEBUGMSG(ZONE_PDD, (_T("Can not prime for ZLT (%s): busy\r\n"),
            bIN ? _T("IN") : _T("OUT")) );
        return;
    }

    memset(&(pPdd->qhbuffer->qh[endp]),0, sizeof(USBD_dQH_T)-2*sizeof(DWORD));
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

        pEpc = (DWORD*)&edptcomp;
        *pEpc = INREG32(&pOtgReg->ENDPTCOMPLETE);
        if ( (bIN && ((edptcomp.ETCE & 1) == 0)) || (!bIN && ((edptcomp.ERCE & 1) == 0)) )
        {
            DEBUGMSG(ZONE_PDD,(_T("################ Failed to prime for ZLT %s\r\n"), bIN ? TEXT("IN") : TEXT("OUT")));
        }
    }

    pPdd->qhbuffer->bPrimed[endp] = TRUE;
}

//-------------------------------------------------------------
//
//  Function: DeviceAttach
//
//  This is to handle the USB Device Attach. There is no interupt
//  for device attach. This function is forced in USB reset.
//
//  Parameters:
//      pPdd - Pointer to USBFN_PDD
//
//  Return: NULL
//
//------------------------------------------------------------------
static void DeviceAttach(USBFN_PDD *pPdd)
{
    if (pPdd->devState==0) // Why no bit set in PORTSC???
    {
        // Let MDD process change
        USB_PORTSC_T state;
        DWORD * t1;
        int i;

        DEBUGMSG(ZONE_PDD, (L"Device Attach\r\n"));

        //
        // Get device state & change
        t1=(DWORD *)&state;
        *t1 = INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[USBD_PORT_NUM]);

        pPdd->devState=1;

        UNLOCK();
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
        LOCK();

        if (state.PSPD==0x2)
        {
            UNLOCK();
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_HIGH_SPEED);

            LOCK();
        }
        else
        {
            UNLOCK();
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
static void HandleUSBReset(USBFN_PDD *pPdd)
{
    volatile CSP_USB_REG * pOtgReg=&(pPdd->pUSBDRegs->OTG);
    USB_PORTSC_T state;
    DWORD * tPortsc;

    LOCK();

    //Device attach need to be handled before reset. There is no interrupt for device attach.
    DeviceAttach(pPdd);

    // clear all setup notification bits
    OUTREG32(&pOtgReg->ENDPTSETUPSTAT, INREG32(&pOtgReg->ENDPTSETUPSTAT));

    // clear all complete bits
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
        DEBUGMSG(ZONE_PDD,(_T("HandleUSBReset: Not still in reset condition. Possibly reset controller??\r\n")));
    }



    pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_RESET);

    UNLOCK();

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

        // Clear USB Interrupt
        pPdd->dwUSBIntrValue = INREG32(&pPdd->pUSBDRegs->OTG.USBINTR);
        OUTREG32(&pPdd->pUSBDRegs->OTG.USBINTR, 0x00);
        DEBUGMSG(ZONE_POWER, (TEXT("PowerDown USBINTR = 0x%x\r\n"), INREG32(&pPdd->pUSBDRegs->OTG.USBINTR)));

        // Clear USBSTS
        OUTREG32(&pPdd->pUSBDRegs->OTG.USBSTS, INREG32(&pPdd->pUSBDRegs->OTG.USBSTS));


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

        // We have to disable the wakeup, or it will fail in following condition:
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
        DEBUGMSG(ZONE_POWER, (TEXT("PowerDown PORTSC= 0x%x\r\n"), INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0])));

        // We need to wait for SUSPEND bit set
        //while((INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]) & 0x80) == 0x00)
        //  Sleep(0);
        if (pPdd->bUSBPanicMode)
        {
            pPdd->bUSBPanicMode = FALSE;
#if 1 /*ERIC0907, need implemented?*/
            DEBUGMSG(TRUE, (TEXT("DDKClockDisablePanicMode : 5\r\n")));
#else
            DDKClockDisablePanicMode();
#endif
        }

        // Stop USB Clock now
        if (pPdd->bUSBCoreClk)
        {
            pPdd->bUSBCoreClk = FALSE;
            USBClockDisable(TRUE);
        }
    }
    DEBUGMSG(ZONE_FUNCTION, (TEXT("PowerDown completed\r\n")));
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

    DEBUGMSG(ZONE_POWER, (_T("UsbPdd_PowerUp with bInUSBFN %d\r\n"), pPdd->bInUSBFN) );

    if (pPdd->bInUSBFN)
    {

        USB_CTRL_T ctrl;
        USB_USBCMD_T usbcmd;
        DWORD *temp3;
        DWORD *temp2;

        // Start USB Clock now
        if (pPdd->bUSBCoreClk == FALSE)
        {
            DEBUGMSG(ZONE_PDD, (TEXT("USB Clock start\r\n")));
            pPdd->bUSBCoreClk = TRUE;
            USBClockDisable(FALSE);
        }

        if (pPdd->bUSBPanicMode == FALSE)
        {
            DEBUGMSG(ZONE_POWER, (TEXT("Panic Mode On\r\n")));
            pPdd->bUSBPanicMode = TRUE;
#if 1 /*ERIC0907 need implemented?*/
            DEBUGMSG(ZONE_POWER, (TEXT("DDKClockEnablePanicMode : 6\r\n")));
#else
            DDKClockEnablePanicMode();
#endif
        }

        DEBUGMSG(ZONE_POWER, (TEXT("PowerUp:USBSTS(0x%x) PORTSC(0x%x)\r\n"),
            INREG32(&pPdd->pUSBDRegs->OTG.USBSTS), INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]) ));
        // Force resume first
        if (INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]) & 0x800000)
        {
            // Clear the PHCD
            CLRREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0], INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0]) & 0x800000);

            Sleep(1);   //A short waiting period is needed here
            // Force Resume bit
            OUTREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0], INREG32(&pPdd->pUSBDRegs->OTG.PORTSC[0])|0x40);

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
        DEBUGMSG(ZONE_POWER, (TEXT("PowerUp USBCMD = 0x%x\r\n"), INREG32(&pPdd->pUSBDRegs->OTG.USBCMD)));


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
    DEBUGMSG(ZONE_FUNCTION, (_T("UpdateDevicePower Going from D%d to D%d\r\n"), pPdd->m_CurActualPowerState , cpsNew));
    if ( (cpsNew < pPdd->m_CurActualPowerState) && pPdd->hParentBus)
    {
        BOOL bBusSucceed = SetDevicePowerState(pPdd->hParentBus, cpsNew, NULL);
        if (bBusSucceed &&
            (pPdd->m_CurActualPowerState==D3 || pPdd->m_CurActualPowerState==D4))
        {
            DEBUGMSG(ZONE_POWER, (TEXT("UpdateDevicePower to powerup\r\n")));
            UfnPdd_PowerUp((PVOID)pPdd);
        }
    }

    if ( (cpsNew > pPdd->m_CurActualPowerState ) && pPdd->hParentBus  )
    {
        BOOL bBusSucceed = SetDevicePowerState(pPdd->hParentBus, cpsNew, NULL);
        if (bBusSucceed && (cpsNew == D4 ||cpsNew == D3 ))
        {
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
//  Parameter :
//      pPdd - Pointer to USBFN_PDD
//
//  Return:
//      NULL
//
//--------------------------------------------------------------------------------
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

        DEBUGMSG(ZONE_PDD, (L"Device Attach\r\n"));

        // Get device state & change
        t1=(DWORD *)&state;
        *t1 = INREG32(&pUSBDRegs->OTG.PORTSC[USBD_PORT_NUM]);

        pPdd->devState=1;

        UNLOCK();
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_ATTACH);
        LOCK();

        if (state.PSPD==0x2)
        {
            UNLOCK();
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_SPEED, BS_HIGH_SPEED);
            LOCK();
        }
        else
        {
            UNLOCK();
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

    pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_SETUP_PACKET, (DWORD)data);
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
        DEBUGMSG(ZONE_PDD, (_T("Device Detach\r\n")));

        // Don't process other changes (we are disconnected)
        pPdd->devState=0;

        UNLOCK();
        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);
        LOCK();

        // SetSelfPowerState to D4
        pPdd->m_CurSelfPowerState = D4; // Do we need set to D3 as wake up source?
        UpdateDevicePower(pPdd);

        OUTREG32(&pPdd->pUSBDRegs->OTG.T_154H.USBADR, 0);
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
//
//      NULL
//
//-------------------------------------------------------------------------------
static void CheckEndpoint(USBFN_PDD *pPdd, int i )
{
    CSP_USB_REGS* pUSBDRegs = pPdd->pUSBDRegs;
    STransfer * pTransfer;
    USB_ENDPTCOMPLETE_T edptcomp;
    DWORD *pEpc;
    DWORD dwIsochTransferred = 0;
    BOOL bIsIsoch = FALSE;
    BOOL dwTraverseState = ISO_TD_LIST_TRAVERSE_COMPLETE;

    pEpc = (DWORD*)&edptcomp;

    LOCK();

    // OUT endpoint
    *pEpc = INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE);

    if (edptcomp.ERCE&(1<<i))
    {
        PUSBD_dTD_R_T pTd;
        PUSBD_dQH_T pQh;

        CacheSync(CACHE_SYNC_DISCARD);


        pTransfer=pPdd->ep[i]->pTransfer;

        bIsIsoch = pPdd->ep[i]->bIsochronous;


        pQh=&(pPdd->qhbuffer->qh[i*2]);
        pTd=TDVirtual(pQh->curr_dTD);

        if (pPdd->qhbuffer->qh[i*2].dtd.status)
            DEBUGMSG(ZONE_PDD,(L"Status is not 0 on recv %x =%x",i,pPdd->qhbuffer->qh[i*2].dtd.status));

        // if the current DTD is not ACTIVE (and active may mean that we are in the middle of traversing the list)
        if (pTransfer && (pPdd->qhbuffer->qh[i*2].dtd.status!=0x80) && (dwTraverseState != ISO_TD_LIST_TRAVERSE_INCOMPLETE) )
        {
            DWORD len;

            if( !bIsIsoch )
            {
                // calculate how much we were trying to tranfer in this TD
                len=pTransfer->cbBuffer-pTransfer->cbTransferred;

                if (len>MAX_SIZE_PER_TD)
                    len=MAX_SIZE_PER_TD;

                len-=pPdd->qhbuffer->qh[i*2].dtd.tb;  // reduce by any bytes outstanding

                // len is how much was actually transferred
                pTransfer->cbTransferred+=len;
            }
            else
            {
                len=pTransfer->cbBuffer-pTransfer->cbTransferred;

                len-=pPdd->qhbuffer->qh[i*2].dtd.tb;  // reduce by any bytes outstanding

                pTransfer->cbTransferred = len;
            }

            // TODO:  do we write ENDPTCOMPLETE when it is a rolling isoch transfer????
            //        (right now we do - but waiting on Freescale to report)
            DEBUGMSG(ZONE_PDD, (TEXT("ENDPTCOMPLETE %x\r\n"),(1<<i)));

            OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), (1<<i));

            if (pPdd->qhbuffer->qh[i*2].dtd.status && pPdd->qhbuffer->qh[i*2].dtd.status!=0x80)
            {
                DEBUGMSG(ZONE_ERROR,(_T("CheckEndpoint(ep=%d): BUFFER_ERROR\r\n"), i) );

                pTransfer->dwUsbError=UFN_CLIENT_BUFFER_ERROR;
            }
            else if (
                len==MAX_SIZE_PER_TD && pTransfer->cbTransferred < pTransfer->cbBuffer )
            {
                pTransfer->dwUsbError=UFN_MORE_DATA;

                UNLOCK();
                UfnPdd_IssueTransfer(pPdd, i,pTransfer);
                LOCK();
            }
            else
            {
                pTransfer->dwUsbError=UFN_NO_ERROR;
            }

            pPdd->qhbuffer->bPrimed[i] = FALSE;

            if (pTransfer->dwUsbError != UFN_MORE_DATA )
            {
                //Data are already in the buffer pointed by pTD. No need to mem copy.

                if ( pPdd->ep[i]->bPagesLocked )
                {
                    DEBUGCHK( pPdd->ep[i]->pMappedBufPtr != NULL );
                    UnlockPages(pPdd->ep[i]->pMappedBufPtr, pTransfer->cbBuffer);
                    pPdd->ep[i]->bPagesLocked = 0;

                    CeCloseCallerBuffer(
                        (LPVOID)pPdd->ep[i]->pMappedBufPtr,
                        (LPVOID)pTransfer->pvBuffer,
                        pTransfer->cbBuffer,
                        ARG_IO_PTR
                        );
                }


                pPdd->ep[i]->pTransfer=NULL;

                if( bIsIsoch )
                {
                    // TODO: only free the whole list like this if there isn't any more to transfer
                    InitIsochEPLinkList( pPdd->ep[i] );
                }

                UNLOCK();
                pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);

                DEBUGMSG(ZONE_PDD, (TEXT("OUT %s Completed for endpoint %d\r\n"),
                                pTransfer->dwUsbError == UFN_NO_ERROR ? TEXT("Normal") : TEXT("Cancel"),
                                i));
                LOCK();
            }
        }
        else //(pTransfer&&pPdd->qhbuffer->qh[i*2].dtd.status!=0x80)
        {
            DEBUGMSG(ZONE_PDD, (TEXT("############# ENDPTCOMPLETE %x NULL ")
                                TEXT("pTransfer or bad status (0x%x)!\r\n"),
                                (1<<i),
                                pPdd->qhbuffer->qh[i*2].dtd.status));

            OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), (1<<i));
        }
    }


    // IN packet transcation
    if (edptcomp.ETCE&(1<<i))
    {
        DWORD mapped_epNum = i;

        if (i == 0)
        {
            /*
            * If this is the completion of a SetAddress (the status handshake part is an "IN") then
            * we should set the address now.  We only set the address after the command.
            * Address is & 0x80 as a flag to tell us to set the address.
            */
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

        if (pTransfer && (pPdd->qhbuffer->qh[i*2+1].dtd.status!=0x80) && (dwTraverseState != ISO_TD_LIST_TRAVERSE_INCOMPLETE) )
        {
            int len;

            if (i) // if endpoint is NOT zero
            {
                PUSBD_dTD_R_T pTd;
                PUSBD_dQH_T pQh;
                bIsIsoch = pPdd->ep[mapped_epNum]->bIsochronous;

                pQh=&(pPdd->qhbuffer->qh[i*2+1]);
                pTd=TDVirtual(pQh->curr_dTD);
            }


            if( !bIsIsoch )
            {
                len=pTransfer->cbBuffer-pTransfer->cbTransferred;

                if (len>MAX_SIZE_PER_TD)
                {
                    len=MAX_SIZE_PER_TD;
                }

                len -= pPdd->qhbuffer->qh[i*2+1].dtd.tb; // reduce by what's still to go in the TD

                pTransfer->cbTransferred += len;
            }
            else
            {
                len=pTransfer->cbBuffer-pTransfer->cbTransferred;

                len-=pPdd->qhbuffer->qh[i*2].dtd.tb;  // reduce by any bytes outstanding

                pTransfer->cbTransferred = len;
            }

            pPdd->qhbuffer->bPrimed[mapped_epNum] = FALSE;
            if (pPdd->qhbuffer->qh[i*2+1].dtd.tb)
            {
                DEBUGMSG(ZONE_PDD, (L"Send error?? len=%x, status=%x, tb=%x\r\n",
                                    pTransfer->cbBuffer,
                                    pPdd->qhbuffer->qh[i*2+1].dtd.status,
                                    pPdd->qhbuffer->qh[i*2+1].dtd.tb));

                pTransfer->cbTransferred=pTransfer->cbBuffer;
            }

            if ( (pTransfer->cbTransferred<pTransfer->cbBuffer)  && (!bIsIsoch) )
            {
                pTransfer->dwUsbError=UFN_MORE_DATA;
                OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), (0x10000UL<<i));

                UNLOCK();
                UfnPdd_IssueTransfer(pPdd, i, pTransfer);
                LOCK();
            }
            else
            {
                pTransfer->dwUsbError=UFN_NO_ERROR;

                OUTREG32(&(pUSBDRegs->OTG.ENDPTCOMPLETE), (0x10000UL<<i));

                if ( pPdd->ep[mapped_epNum]->bPagesLocked )
                {
                    DEBUGCHK( pPdd->ep[mapped_epNum]->pMappedBufPtr != NULL );
                    UnlockPages(pPdd->ep[mapped_epNum]->pMappedBufPtr, pTransfer->cbBuffer);
                    pPdd->ep[mapped_epNum]->bPagesLocked = 0;

                    CeCloseCallerBuffer(
                        (LPVOID)pPdd->ep[mapped_epNum]->pMappedBufPtr,
                        (LPVOID)pTransfer->pvBuffer,
                        pTransfer->cbBuffer,
                        ARG_IO_PTR
                        );
                }

                pPdd->ep[mapped_epNum]->pTransfer=NULL;

                if( bIsIsoch )
                {
                    // TODO: only free the whole list like this if there isn't any more to transfer
                    InitIsochEPLinkList( pPdd->ep[mapped_epNum] );
                }

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
*       Output:
*            void
*
*       Return:
*            ERROR_SUCCESS - okay, mode set
*            ERROR_GEN_FAILURE -- couldn't set the mode for some reason
*
*
*       Function waits until controller is stopped or started.
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
        cmd.RS = 1;
    }
    else
    {
        cmd.RS = 0;
    }

    OUTREG32(&pRegs->OTG.USBCMD,*pTmpCmd);

    *pTmpMode = INREG32(&pRegs->OTG.USBMODE);
    if ( mode.CM == 0x3 )
    {
        // in host mode, make sure HCH (halted) already, and that mode does change to halted (or not)
        USB_USBSTS_T stat;
        DWORD *pTmpStat = (DWORD*)&stat;
        int iAttempts;

        if ( bRunMode )
        {
            *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
            if ( stat.HCH == 0)
            {
                return ERROR_GEN_FAILURE;
            }
        }

        // wait for mode to change
        iAttempts = 0;
        do {
            *pTmpStat = INREG32(&pRegs->OTG.USBSTS);
            if ( (!bRunMode && stat.HCH) || (bRunMode && (stat.HCH == 0)) )
                return ERROR_SUCCESS;

            Sleep(1);
        } while ( iAttempts++ < 50 );

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
//--------------------------------------------------------------------------------
static DWORD WINAPI InterruptThread(VOID *pPddContext)
{
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS* pUSBDRegs = pPdd->pUSBDRegs;
    USB_USBSTS_T source;
    DWORD *temp;
    int i;
    ULONG WaitReturn;
    TCHAR szUSBFunctionObjectName[30];
    TCHAR szUSBXcvrObjectName[30];
    TCHAR szUSBTransferEventName[30];
    DWORD timeout = IDLE_TIMEOUT; // 3 sec

    DEBUGMSG(ZONE_FUNCTION, (TEXT("IsOTGSupport? %d\r\n"), pPdd->IsOTGSupport));
    if (pPdd->IsOTGSupport)
    {
        StringCbCopy(szUSBFunctionObjectName, sizeof(szUSBFunctionObjectName), USBFunctionObjectName);
        StringCbCat(szUSBFunctionObjectName, sizeof(szUSBFunctionObjectName), pPdd->szOTGGroup);

        pPdd->hFunctionEvent = CreateEvent(NULL, FALSE, FALSE, szUSBFunctionObjectName);
        if (GetLastError() == ERROR_ALREADY_EXISTS)
            DEBUGMSG(ZONE_PDD, (TEXT("UFN: Opened an existing Func Event\r\n")));
        else
            DEBUGMSG(ZONE_PDD, (TEXT("UFN: Created a new Func Event\r\n")));
        if (pPdd->hFunctionEvent == NULL)
            DEBUGMSG(ZONE_PDD, (TEXT("UFN: Create Event Failed for func!\r\n")));

        StringCbCopy(szUSBXcvrObjectName, sizeof(szUSBXcvrObjectName), USBXcvrObjectName);
        StringCbCat(szUSBXcvrObjectName, sizeof(szUSBXcvrObjectName), pPdd->szOTGGroup);

        pPdd->hXcvrEvent = CreateEvent(NULL, FALSE, FALSE, szUSBXcvrObjectName);

        if (GetLastError() == ERROR_ALREADY_EXISTS)
            DEBUGMSG(ZONE_PDD, (TEXT("UFN: Opened an existing XCVR Event\r\n")));
        else
            DEBUGMSG(ZONE_PDD, (TEXT("UFN: Created a new XCVR Event\r\n")));
        if (pPdd->hXcvrEvent == NULL)
            DEBUGMSG(ZONE_PDD, (TEXT("UFN: Create Event Failed for xcvr!\r\n")));

        // Event created for transferring control to transceiver when function controller is unloaded
        StringCbCopy(szUSBTransferEventName, sizeof(szUSBTransferEventName), USBTransferEventName);
        StringCbCat(szUSBTransferEventName, sizeof(szUSBTransferEventName), pPdd->szOTGGroup);

        pPdd->hTransferEvent = CreateEvent(NULL, FALSE, FALSE, szUSBTransferEventName);

        if (GetLastError() == ERROR_ALREADY_EXISTS)
            DEBUGMSG(ZONE_PDD, (TEXT("UFN: Opened an existing Transfer Event\r\n")));
        else
            DEBUGMSG(ZONE_PDD, (TEXT("UFN: Created a new Transfer Event\r\n")));
        if (pPdd->hTransferEvent == NULL)
            DEBUGMSG(ZONE_PDD, (TEXT("UFN: Create Event Failed for Transfer!\r\n")));

XCVR_SIG:
        pPdd->bInUSBFN = FALSE;

        WaitReturn = WaitForSingleObject(pPdd->hFunctionEvent, INFINITE);
        // This check is required if function event is set from deinit
        if (pPdd->exitIntrThread)
        {
            goto cleanup;
        }

        if(!InterruptInitialize(pPdd->sysIntr, pPdd->hIntrEvent, NULL, 0))
        {
            DEBUGMSG(ZONE_FUNCTION, (L"ERROR: UfnPdd_Init: Interrupt initialization failed\r\n"));
            return -1;
        }
        pPdd->bInUSBFN = TRUE;
        UfnPdd_Start(pPdd);
        // Move this part to here so that it would only handle in case
        // of OTG support
        {
            USB_OTGSC_T temp;
            DWORD       *t = (DWORD *)&temp;

            *t = INREG32(&pUSBDRegs->OTG.OTGSC);
            temp.IDIE = 1;
            OUTREG32(&pUSBDRegs->OTG.OTGSC, *t);
        }

        // Handling of device attach
        temp=(DWORD *)&source;
        *temp = INREG32(&pUSBDRegs->OTG.USBSTS);
        // Clear source bit
        {
            do {
                if (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT) & 1)
                {
                    SetupEvent(pPdd);
                }

                if (INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE))
                {
                    for (i=0;i<USBD_EP_COUNT;i++)
                    {
                        CheckEndpoint(pPdd, i );
                    }
                }
            } while (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT) ||INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE));
        }
    }

    while (!pPdd->exitIntrThread)
    {
        DWORD dwErr = 0;

        if (pPdd->IsOTGSupport)
            timeout = INFINITE;

        // Wait for interrupt from USB controller
        dwErr = WaitForSingleObject(pPdd->hIntrEvent, timeout);
        if (dwErr == WAIT_TIMEOUT)
        {
            // Now I am ready to put the transceiver into suspend mode.
            // But be aware there is a device attach when boot up
            USB_CTRL_T ctrl;
            DWORD *temp3;
            USB_PORTSC_T portsc;
            DWORD *temp = (DWORD *)&portsc;

            *temp = INREG32(&(pUSBDRegs)->OTG.PORTSC);
            if ((portsc.SUSP == 0) && (pPdd->devState == 1))
            {
                // there is a device attached, don't do anything
                timeout = INFINITE;
                continue;
            }

            // If there is a device attached at this time
            // Handling of device attach
            // Even though it is pure client, still, we have no other way to isolate the
            // host from the device, we need to use ID pin to detect.
            if (portsc.SUSP == 0)
            {
                USB_OTGSC_T otgsc;
                DWORD *temp4 = (DWORD *)&otgsc;

                *temp4 = INREG32(&(pUSBDRegs)->OTG.OTGSC);
                timeout = IDLE_TIMEOUT;
                if (otgsc.ID != 0)
                {
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

#if (USB_CLIENT_MODE == 0)
            ctrl.OUIE = 1;
#else
            ctrl.OUIE = 0; // for full speed  - serial
            ctrl.OSIC = 0x1; //OWIE_INT_ENABLE , for full speed;
#endif
            OUTREG32(&pUSBDRegs->USB_CTRL, *temp3);

            // stop the controller before accessing the ULPI
            LOCK();
            USBControllerRun(pUSBDRegs, FALSE);

            // May need to set ULPI here
            // It is not waking up now ...must be something wrong
            *temp = INREG32(&(pUSBDRegs)->OTG.PORTSC);

#if (USB_CLIENT_MODE == 0)
            if (portsc.PHCD == 0)
                SetULPIToClientMode(pUSBDRegs);
#else
            portsc.PTS = 0x3; // for full speed
#endif
            portsc.PHCD = 1;
            OUTREG32(&(pUSBDRegs)->OTG.PORTSC, *temp);

            {
                BOOL bTemp;

                if (pPdd->bUSBPanicMode == TRUE)
                {
#if 1 /*ERIC0907, need implemented?*/
                    bTemp = TRUE;
                    DEBUGMSG(TRUE, (TEXT("DDKClockDisablePanicMode : 7\r\n")));
#else
                    bTemp = DDKClockDisablePanicMode();
#endif
                    pPdd->bUSBPanicMode = FALSE;
                }
            }

            if (pPdd->bUSBCoreClk == TRUE)
            {
                pPdd->bUSBCoreClk = FALSE;
                USBClockDisable(TRUE);
            }
            UNLOCK();
            // Now we can stop the USB clock
            DEBUGMSG(ZONE_PDD, (TEXT("PDD - SUSPEND\r\n")));
            timeout = INFINITE;
            continue;
        }

PROCESS_INTERRUPT:
        LOCK();
        if (pPdd->bUSBCoreClk == FALSE)
        {
            USBClockDisable(FALSE);
            pPdd->bUSBCoreClk = TRUE;
        }

        if (pPdd->bUSBPanicMode == FALSE)
        {
#if 1 /*ERIC0907, need implemented?*/
            DEBUGMSG(TRUE, (TEXT("DDKClockEnablePanicMode : 8\r\n")));
#else
            DDKClockEnablePanicMode();
#endif
            pPdd->bUSBPanicMode = TRUE;
            DEBUGMSG(ZONE_PDD, (TEXT("DDKClockEnablePanicMode\r\n")));
            DEBUGMSG(ZONE_PDD, (TEXT("WakeUp - PORTSC: 0x%x\r\n"), INREG32(&pUSBDRegs->OTG.PORTSC[0])));
        }

        if (INREG32(&pUSBDRegs->OTG.PORTSC[0]) & 0x800000)
        {
            // Clear the PHCD
            CLRREG32(&pUSBDRegs->OTG.PORTSC[0], INREG32(&pUSBDRegs->OTG.PORTSC[0]) & 0x800000); //PORTSC_PHCD 0x800000
            // Force Resume bit
            OUTREG32(&pUSBDRegs->OTG.PORTSC[0], INREG32(&pUSBDRegs->OTG.PORTSC[0]) | 0x40); //PORTSC_FPR 0x40
        }
        UNLOCK();

        InterruptDone(pPdd->sysIntr);

        CeLogMsg(_T("Intr:Signalled"));

        {
            USB_CTRL_T ctrl;
            temp = (DWORD *)&ctrl;
            *temp = INREG32(&pUSBDRegs->USB_CTRL);

            if (ctrl.OWIR == 1)
            {
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
cleanup:
            //Close handle for function event
            if (pPdd->hFunctionEvent != NULL)
            {
                CloseHandle(pPdd->hFunctionEvent);
                pPdd->hFunctionEvent = NULL;
            }

            //Close handle for transeiver event
            if (pPdd->hXcvrEvent != NULL)
            {
                CloseHandle(pPdd->hXcvrEvent);
                pPdd->hXcvrEvent = NULL;
            }

            //Close handle for transfer event
            if (pPdd->hTransferEvent != NULL)
            {
                CloseHandle(pPdd->hTransferEvent);
                pPdd->hTransferEvent = NULL;
            }
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
            * when device is in test mode, there is no longer any more normal operation and
            * we could just busy-stall here, since exit from the mode is via power-off.
            */
            pPdd->exitIntrThread = TRUE;
            UNLOCK();
            break;
        }

        if (pPdd->m_CurSelfPowerState == D3 || pPdd->m_CurSelfPowerState == D4)
        {
            pPdd->m_CurSelfPowerState = D2;
            UpdateDevicePower(pPdd);
        }


        temp=(DWORD *)&source;
        // Get interrupt source
        *temp = INREG32(&pUSBDRegs->OTG.USBSTS);

        do
        {
            // Clear the reg
            OUTREG32(&pUSBDRegs->OTG.USBSTS, *temp);

            if (source.URI)
            {
                HandleUSBReset(pPdd);
            }

            if (source.SLI)
            {
                CeLogMsg(_T("Intr:SLI"));
                LOCK();
                DevStatEvent(pPdd); // Handle device state change
                if ((pPdd->devState == 0) && (pPdd->IsOTGSupport == 0))
                    timeout = IDLE_TIMEOUT;
                UNLOCK();
            }

            // USB Interrupt
            if (source.UI)
            {
                CeLogMsg(_T("Intr:UI SETUPSTAT %x ENDPTCOMP %x"),
                    INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT),
                    INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE) );

                while (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT) || INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE))
                {
                    if (INREG32(&pUSBDRegs->OTG.ENDPTSETUPSTAT))
                    {
                        SetupEvent(pPdd);  // locks internally, and checks setupstat internally
                    }
                    if (INREG32(&pUSBDRegs->OTG.ENDPTCOMPLETE))
                    {
                        for (i=0;i<USBD_EP_COUNT;i++)
                        {
                            // check endpoint sees if endptcomplete bit is set for individual endpoint i
                            CheckEndpoint(pPdd, i );  // locks internally, and checks regs internally
                            // and clears the endptcomplete bit for that endpoint
                        }
                    }
                }
            }


            *temp = INREG32(&pUSBDRegs->OTG.USBSTS);

            if ((*temp & 0xffffff6f)==0)
            {
                CeLogMsg(_T("Intr:Done"));

                //CheckForEventLost(pPdd,k);

                break;
            }

        }  while (1);

        if (pPdd->bResume)
        {
            CeLogMsg(_T("Resume"));

            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_BUS_EVENTS, UFN_DETACH);

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
                // wait for client function class and make sure everything is disconnected
                // this is not a very good design but since we need to make sure we can get proper
                // disconnect before we switch back to XVR, we have to do that especially on Active Sync.
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

            temp2 = (DWORD *)&state;
            *temp2 = INREG32(&pUSBDRegs->OTG.PORTSC[0]);

            temp3 = (DWORD *)&otgsc;
            *temp3 = INREG32(&pUSBDRegs->OTG.OTGSC);

            CeLogMsg(_T("Detach: PORTSC 0x%x"), *temp2);

            if ((state.SUSP == 1) || (otgsc.ID == 0))
            {
                if (otgsc.IDIS)
                {
                    // clear the register. since other is not used, we can simply
                    // use the SETREG32 instead of OUTREG32
                    *temp3 = 0;
                    otgsc.IDIS = 1;
                    SETREG32(&pUSBDRegs->OTG.OTGSC, *temp3);
                }
                UNLOCK();
                InterruptDisable(pPdd->sysIntr);
                SetEvent(pPdd->hXcvrEvent);
                goto XCVR_SIG;
            }
        }

        if ((pPdd->IsOTGSupport == 0) && (pPdd->devState == 0))
            timeout = IDLE_TIMEOUT;

        UNLOCK();
    }

    if (pPdd->bEnterTestMode)
    {
        pPdd->bEnterTestMode = FALSE;
        while(1);
    }

    return ERROR_SUCCESS;
}


//-----------------------------------------------------------------
//
//  Function: BuildTdBufferList
//
//  Given a mapped endpoint and a TransferDescriptor and a length of data to put into it,
//  build the list of buffer pointers from previously locked page data (including current page).
//  ep is current endpoint, pTd points to the TD being built in uncached mem
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
static void BuildTdBufferList( USBFN_EP *ep, PUSBD_dTD_R_T pTd, DWORD len, BOOL InTransfer )
{
    DWORD dwOffset;
    DWORD dwPageIdx=0;

    if ( len > 0 )
    {
        dwPageIdx = ep->dwNextPageIdx;
        DEBUGCHK( dwPageIdx < ep->dwNumPages );


        pTd->bp0 = (ep->pdwPageList[dwPageIdx++] << UserKInfo[KINX_PFN_SHIFT]) >> 12;

        // get current buffer pointer's offset into first page
        pTd->curr_off = dwOffset = ((DWORD)ep->pMappedBufPtr) & (UserKInfo[KINX_PAGESIZE] - 1);

        /* the first buffer page will contain either one MAX_SIZE_PER_BP or less (if there was an offset) */
        if ( len > (MAX_SIZE_PER_BP - dwOffset) )
            len -= (MAX_SIZE_PER_BP - dwOffset);
        else
            len = 0;   // first bp contains all the data
    }

    // continue to buffer page 1
    if ( len > 0 )
    {
        DEBUGCHK( dwPageIdx < ep->dwNumPages );

        pTd->bp1 = (ep->pdwPageList[dwPageIdx++] << UserKInfo[KINX_PFN_SHIFT])>>12;

        // each of the following pages will start at offset 0
        if ( len > MAX_SIZE_PER_BP )
        {
            len -= MAX_SIZE_PER_BP;
        }
        else
        {
            len = 0;
        }
    }

    // continue to buffer page 2
    if ( len > 0 )
    {
        DEBUGCHK( dwPageIdx < ep->dwNumPages );

        pTd->bp2 = (ep->pdwPageList[dwPageIdx++] << UserKInfo[KINX_PFN_SHIFT])>>12;

        if ( len > MAX_SIZE_PER_BP )
        {
            len -= MAX_SIZE_PER_BP;
        }
        else
        {
            len = 0;
        }
    }

    // continue to buffer page 3
    if ( len > 0 )
    {
        DEBUGCHK( dwPageIdx < ep->dwNumPages );

        pTd->bp3 = (ep->pdwPageList[dwPageIdx++] << UserKInfo[KINX_PFN_SHIFT])>>12;

        if ( len > MAX_SIZE_PER_BP )
        {
            len -= MAX_SIZE_PER_BP;
        }
        else
        {
            len = 0;
        }
    }

    // continue to buffer page 4
    if (  len > 0 )
    {
        DEBUGCHK( dwPageIdx < ep->dwNumPages );

        pTd->bp4 = (ep->pdwPageList[dwPageIdx] << UserKInfo[KINX_PFN_SHIFT])>>12;

        /* whenever we extend into this fifth BP, we are either completely finished
        * with all our data, or next time we should start with the same page (from
        * the current offset
        */
    }

    ep->dwNextPageIdx = dwPageIdx;
}

//------------------------------------------------------------------------------
//
//  Function:  IssueTransfer_ISO
//
//  Issues Isochronous transfers
//
//  Parameter:
//
//  Returns:  TRUE for success,  FALSE for error.
//            IF return is FALSE, then pdwTransferResult is set to the error code
//            IF return is true, then transfer was issued okay, and the result is deferred
//------------------------------------------------------------------------------
static BOOL IssueTransfer_ISO( USBFN_PDD *pPdd, DWORD mapped_epNum, STransfer *pTransfer, BOOL  bFirstTimeThrough, DWORD *pdwTransferResult )
{
    DWORD bytesToAdd = 0;
    DWORD bytesPerTd = 0;
    DWORD dwTdsUsed;
    USBD_dTD_R_T *pElementsToAdd;
    USBD_dTD_R_T *pPriorTd;
    USBD_dTD_R_T *pCurrentTd;
    DWORD dwPhysicalAddress;
    PUSBD_dQH_T pQh;
    USBFN_EP *pEndpoint = pPdd->ep[mapped_epNum];
    BOOL bOk;

    bytesPerTd = pEndpoint->maxPacketSize;
    *pdwTransferResult = UFN_NO_ERROR; // for now

    if (bFirstTimeThrough)
    {
        /*
         * Issue Transfer can be called several times per transfer.
         * This is the first time we've called this function for this transfer
         */
        pTransfer->cbTransferred=0;  // so nothing transferred yet

        // zero out all the information that is only for NON-ISO endpoints
        pEndpoint->dwNextPageIdx = 0;
        pEndpoint->pMappedBufPtr = NULL;
        memset( pEndpoint->pdwPageList, 0, sizeof(pEndpoint->pdwPageList) );
        pEndpoint->bPagesLocked = 0;
        pEndpoint->dwNumPages = 0;
        pEndpoint->fZeroLengthNeeded = FALSE; // no support for zero-length isoch transfers

        if ( pTransfer->pvBuffer )
        {
            /* user provided a buffer so we need to lock it and get the physical pages */

            bOk = FALSE;
            if (FAILED(CeOpenCallerBuffer(
                (PVOID *)&(pPdd->ep[mapped_epNum]->pMappedBufPtr),
                (LPVOID)pTransfer->pvBuffer,
                pTransfer->cbBuffer,
                ARG_IO_PTR,
                FALSE)))
            {
                DEBUGMSG(ZONE_ERROR,
                    (_T("UfnPdd_IssueTransfer: Failed to map buffer pointer (0x%x, %d bytes) to caller\r\n"),
                    pTransfer->pvBuffer,
                    pTransfer->cbBuffer
                    ));
            }
            else
            {
                if ( pTransfer->cbBuffer > 0 )
                {
                    bOk = LockPages(pPdd->ep[mapped_epNum]->pMappedBufPtr, pTransfer->cbBuffer,
                        pPdd->ep[mapped_epNum]->pdwPageList,
                        TRANSFER_IS_IN(pTransfer) ? LOCKFLAG_READ : LOCKFLAG_WRITE );
                }
            }

            // could check direction of transfer and just lock read, but do this for now

            DEBUGCHK(bOk || (pTransfer->cbBuffer == 0));

            if ( !bOk )
            {
                DWORD dwGLE;

                if ( pTransfer->cbBuffer > 0 )
                {
                    dwGLE = GetLastError();
                    DEBUGMSG(ZONE_ERROR, (_T("UfnPdd_IssueTransfer: Buffer Page Lock Failed. Error:\r\n"), dwGLE));

                    pPdd->ep[mapped_epNum]->pMappedBufPtr = NULL;
                    pPdd->ep[mapped_epNum]->bPagesLocked = FALSE;
                    pPdd->ep[mapped_epNum]->pTransfer = NULL;
                    pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;

                    UNLOCK();

                    return ERROR_GEN_FAILURE;
                }

                // otherwise, it's a normal situation for null transfer on user-level
            }
            else
            {
                // lock pages success
                pPdd->ep[mapped_epNum]->bPagesLocked = (pTransfer->cbBuffer > 0);
                pPdd->ep[mapped_epNum]->dwNextPageIdx = 0;
                pPdd->ep[mapped_epNum]->dwNumPages =
                    ADDRESS_AND_SIZE_TO_SPAN_PAGES(pTransfer->pvBuffer, pTransfer->cbBuffer);
            }
        }
    }

    bytesToAdd = pTransfer->cbBuffer;

    DEBUGCHK(pTransfer->dwUsbError == UFN_NOT_COMPLETE_ERROR);

    dwPhysicalAddress = (DWORD)pEndpoint->pMappedBufPtr;

    // building linklist starts==========================
    // head of the new list
    pElementsToAdd = pCurrentTd = pEndpoint->pFirstFreeTd;
    pPriorTd = NULL;
    dwTdsUsed = pEndpoint->dwNumTdsUsed;

    while( (bytesToAdd > 0) && (dwTdsUsed < USB_ISOCH_TD_ENTRIES) )
    {
        if( pPriorTd ) // if not first item
        {
            // link this item in
            pPriorTd->T = 0;
            pPriorTd->next_dtd = (DescriptorPhy(pPdd, (DWORD)pCurrentTd)>>5 );
            pPriorTd->ioc = 0; // not the last one   - so we DON'T want this one to interrupt
        }
        // else this is the first item, so we'll link it after this loop, when adding the list

        memset(pCurrentTd, 0, sizeof(USBD_dTD_T));
        if ( bytesToAdd > bytesPerTd )
        {
            pCurrentTd->tb =  bytesPerTd;
        }
        else
        {
            pCurrentTd->tb = bytesToAdd;
        }
        pCurrentTd->status = 0x80;            // Active
        //pCurrentTd->bp0 = (dwPhysicalAddress << UserKInfo[KINX_PFN_SHIFT]) >> 12;
        pCurrentTd->bp0 = (pEndpoint->pdwPageList[0] << UserKInfo[KINX_PFN_SHIFT]) >> 12;

        // get current buffer pointer's offset into first page
        pCurrentTd->curr_off = ( dwPhysicalAddress & (UserKInfo[KINX_PAGESIZE] - 1) );
        if( TRANSFER_IS_IN(pTransfer) )
        {
            pCurrentTd->MultO = 1;
        }

        dwPhysicalAddress += bytesPerTd;
        pPriorTd = pCurrentTd;
        INC_TD_PTR( pEndpoint, pCurrentTd ); // increment pCurrentTd in the circular list
        if ( bytesToAdd < bytesPerTd)
        {
            bytesToAdd = 0;
        }
        else
        {
            bytesToAdd -= bytesPerTd;
        }
        ++dwTdsUsed;
    }


    if( pPriorTd == NULL ) // if we didn't add anything
    {
        // then we either have no space in the buffer
        *pdwTransferResult = UFN_CLIENT_BUFFER_ERROR;
        DEBUGMSG(ZONE_ERROR,(_T("UfnPdd: Unable to start ISO transfer, bytesToAdd=%d, dwTdsUsed=%d\r\n"), bytesToAdd, dwTdsUsed));
        return FALSE;
    }

    pPriorTd->next_dtd = 0xDEAD;
    pPriorTd->T = 1; // terminate list
    pPriorTd->ioc = 1; // this is the last element - we want it to interrupt


    // building linklist ends=====================================================================

    // right here, we have a nicely constructed list,
    // it starts out at pElementsToAdd and it ends at pPriorTd.

    // get the right QH pointer
    if( TRANSFER_IS_IN(pTransfer) )
    {
        pQh=&(pPdd->qhbuffer->qh[mapped_epNum*2+1]);
    }
    else
    {
        pQh=&(pPdd->qhbuffer->qh[mapped_epNum*2]);
    }


    // TODO: link for the LAST USED element and avoid priming
    //       in case the list is not empty

    // Assume only one dTD for one endpoint(except 0) at anytime.
    {
        USB_ENDPTPRIME_T edptprime;
        DWORD * temp=(DWORD * )&edptprime;

        *temp=0;
        if (TRANSFER_IS_IN(pTransfer))
        {
            edptprime.PETB=1<<mapped_epNum;
        }
        else
        {
            edptprime.PERB=1<<mapped_epNum;
        }

        while (INREG32(&pPdd->pUSBDRegs->OTG.ENDPTPRIME))
        {
            ;    // Wait until the last prime is finished
        }

        if((INREG32(&pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE)&(*temp))!=0)
        {
            OUTREG32(&(pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE), *temp);
        }

        memset(pQh, 0, sizeof(USBD_dQH_T));
        pQh->ios = 0;
        pQh->dtd.next_dtd=(DescriptorPhy(pPdd, (DWORD)pElementsToAdd)>>5);
        pQh->zlt = 0;
        pQh->mpl = pEndpoint->maxPacketSize;
        pQh->mult = 1;

        CacheSync(CACHE_SYNC_DISCARD);

        // start the prime
        OUTREG32(&pPdd->pUSBDRegs->OTG.ENDPTPRIME,*temp);
    }

    // finally, success
    pPdd->qhbuffer->bPrimed[mapped_epNum] = TRUE;
    if( pEndpoint->dwNumTdsUsed == 0 ) // if the list is currently empty
    {
        pEndpoint->pFirstUsedTd = pElementsToAdd;
    }
    pEndpoint->pFirstFreeTd = pCurrentTd;
    pEndpoint->dwNumTdsUsed = dwTdsUsed;

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  UfnPdd_IssueTransfer
//
//  This is called by MDD to issue transfer to USB host
//
//  Parameter:
//
//      pPddContext - Pointer to USBFN_PDD
//      endPoint - endpoint to be transferred
//      pTransfer - pointer to STransfer
//
//  Return:
//      ERROR_SUCCESS
//
//--------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_IssueTransfer(
                                  VOID *pPddContext, DWORD endPoint, STransfer *pTransfer
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

    //DEBUG ONLY
    UCHAR* pbuffer = NULL;

    LOCK();

    // Save transfer for interrupt thread
    // Select EP
    epNum = USBD_EP_NUM & endPoint;

    mapped_epNum = epNum;
    if ((pTransfer && TRANSFER_IS_IN(pTransfer)) && (epNum == 0))
    {
        /*
        * For IN EP0, we map to ep0 to the last EP instead.
        * This isn't strictly necessary but was introduced early in development due to
        * restrictions at that time.
        * IN  EP0 => USBD_EP_COUNT-1
        * OUT EP0 => 0
        */
        mapped_epNum = USBD_EP_COUNT-1;
    }

    CeLogMsg(_T("IssueTransfer: %s with %d bytes"),
        pTransfer ? ( TRANSFER_IS_IN(pTransfer) ? _T("IN") : _T("OUT") ) : _T("NULL"),
        pTransfer ? pTransfer->cbBuffer : 0 );


    if (pPdd->ep[mapped_epNum]->pTransfer && pPdd->ep[mapped_epNum]->pTransfer == pTransfer )
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

    if (!TRANSFER_IS_IN(pTransfer)||epNum)
    {
        pPdd->ep[epNum]->pTransfer = pTransfer;
    }
    else
    {
        pPdd->ep[USBD_EP_COUNT-1]->pTransfer = pTransfer; // IN transaction for EP 0

    }

    if (pTransfer->pvBuffer == NULL ) // Sync Length with buffer.
    {
        pTransfer->cbBuffer = 0 ;
    }

    if( pPdd->ep[mapped_epNum]->bIsochronous )
    {
        DWORD  dwTransferResult;
        if( !IssueTransfer_ISO( pPdd, mapped_epNum, pTransfer, bFirstTimeThrough, &dwTransferResult ) )
        {
            pTransfer->dwUsbError = dwTransferResult;
            RETAILMSG(1,
                (_T("IssueTransfer_ISO failed, %d\r\n"),
                dwTransferResult
                ));
            // we couldn't even start the transfer - so we claim it is complete
            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
        }
        UNLOCK();
        return ERROR_SUCCESS;
    }

    if (bFirstTimeThrough)
    {
        /*
        * Issue Transfer can be called several times per transfer.
        * This is the first time we've called this function for this transfer
        */
        pTransfer->cbTransferred=0;  // so nothing transferred yet

        pPdd->ep[mapped_epNum]->dwNextPageIdx = 0;

        if ( (pTransfer->pvBuffer) && (pTransfer->cbBuffer) )
        {
            /* user provided a buffer so we need to lock it and get the physical pages */

            bOk = FALSE;
            if (FAILED(CeOpenCallerBuffer(
                (PVOID *)&(pPdd->ep[mapped_epNum]->pMappedBufPtr),
                (LPVOID)pTransfer->pvBuffer,
                pTransfer->cbBuffer,
                ARG_IO_PTR,
                FALSE)))
            {
                DEBUGMSG(ZONE_ERROR,
                    (_T("UfnPdd_IssueTransfer: Failed to map buffer pointer (0x%x, %d bytes) to caller\r\n"),
                    pTransfer->pvBuffer,
                    pTransfer->cbBuffer
                    ));

                return ERROR_GEN_FAILURE;

            }
            else
            {
                if ( pTransfer->cbBuffer > 0 )
                {
                    bOk = LockPages(pPdd->ep[mapped_epNum]->pMappedBufPtr, pTransfer->cbBuffer,
                        pPdd->ep[mapped_epNum]->pdwPageList,
                        TRANSFER_IS_IN(pTransfer) ? LOCKFLAG_READ : LOCKFLAG_WRITE );
                }
            }

            // could check direction of transfer and just lock read, but do this for now

            DEBUGCHK(bOk || (pTransfer->cbBuffer == 0));

            if ( !bOk )
            {
                DWORD dwGLE;

                if ( pTransfer->cbBuffer > 0 )
                {
                    dwGLE = GetLastError();
                    DEBUGMSG(ZONE_ERROR, (_T("UfnPdd_IssueTransfer: Buffer Page Lock Failed. Error:\r\n"), dwGLE));

                    //Release the marshalled buffer pointer
                    CeCloseCallerBuffer(
                    (LPVOID)pPdd->ep[mapped_epNum]->pMappedBufPtr,
                    (LPVOID)pTransfer->pvBuffer,
                    pTransfer->cbBuffer,
                    ARG_IO_PTR
                    );

                    pPdd->ep[mapped_epNum]->pMappedBufPtr = NULL;
                    pPdd->ep[mapped_epNum]->bPagesLocked = FALSE;
                    pPdd->ep[mapped_epNum]->pTransfer = NULL;
                    pTransfer->dwUsbError = UFN_CLIENT_BUFFER_ERROR;

                    UNLOCK();

                    pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);

                    return ERROR_GEN_FAILURE;
                }

                // otherwise, it's a normal situation for null transfer on user-level
            }
            else
            {
                // lock pages success
                pPdd->ep[mapped_epNum]->bPagesLocked = (pTransfer->cbBuffer > 0);
                pPdd->ep[mapped_epNum]->dwNextPageIdx = 0;
                pPdd->ep[mapped_epNum]->dwNumPages =
                    ADDRESS_AND_SIZE_TO_SPAN_PAGES(pTransfer->pvBuffer, pTransfer->cbBuffer);
            }
        }
    }

    len=pTransfer->cbBuffer-pTransfer->cbTransferred;

    /* we use a one TD for each portion of the transfer, for now.
    * calculate how much we can transfer in this portion.
    * There are 5 page pointers per TD, and first (or last)
    *   might contain less than a full page.
    * if we use a round figure for len then we end up with even number of maxsize packets per TD
    */

    if ( len > MAX_SIZE_PER_TD )
        len = MAX_SIZE_PER_TD;

    DEBUGCHK(pTransfer->dwUsbError == UFN_NOT_COMPLETE_ERROR);

    if (TRANSFER_IS_IN(pTransfer))
    {
        pPdd->ep[mapped_epNum]->fZeroLengthNeeded = (pTransfer->cbBuffer==0) ;
        pTd=&(pPdd->qhbuffer->td[epNum*2+1]);
        pQh=&(pPdd->qhbuffer->qh[epNum*2+1]);

        if (epNum==0)
            pPdd->NSend0ACK=1;
    }
    else
    {
        pPdd->ep[mapped_epNum]->fZeroLengthNeeded = (pTransfer->cbBuffer==0) ;
        pTd=&(pPdd->qhbuffer->td[epNum*2]);
        pQh=&(pPdd->qhbuffer->qh[epNum*2]);
    }

    memset(pTd, 0, sizeof(USBD_dTD_T));
    pTd->T=1;
    pTd->next_dtd=0xDEAD;
    pTd->tb=len;    // Assume cbBuffer <0x1000
    pTd->ioc=1;
    pTd->status=0x80;            // Active

    /* point up to 5 buffer pointers at the current pages
    * we have one static TD right now.
    */

    BuildTdBufferList( pPdd->ep[mapped_epNum], pTd, len, TRANSFER_IS_IN(pTransfer) );

    // Assume only one dTD for one endpoint(except 0) at anytime.
    {
        USB_ENDPTPRIME_T edptprime;
        DWORD * temp=(DWORD * )&edptprime;

        *temp=0;
        if (TRANSFER_IS_IN(pTransfer))
            edptprime.PETB=1<<epNum;
        else
            edptprime.PERB=1<<epNum;


        //dwTimeout = GetTickCount() + USB_TIMEOUT;
        //while (INREG32(&pPdd->pUSBDRegs->OTG.ENDPTPRIME) && (dwTimeout > GetTickCount()))
        while (INREG32(&pPdd->pUSBDRegs->OTG.ENDPTPRIME))
        {
            ;    // Wait until the last prime is finished
        }

        if((INREG32(&pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE)&(*temp))!=0)
        {
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
            // SHOULD result in an immediate interrupt to check setup packet status
            SetEvent(pPdd->hIntrEvent);

            // unmap pages
            if ( pPdd->ep[mapped_epNum]->bPagesLocked )
            {
                UnlockPages(pPdd->ep[mapped_epNum]->pMappedBufPtr, pTransfer->cbBuffer);

                CeCloseCallerBuffer(
                    (LPVOID)pPdd->ep[mapped_epNum]->pMappedBufPtr,
                    (LPVOID)pTransfer->pvBuffer,
                    pTransfer->cbBuffer,
                    ARG_IO_PTR
                    );
            }

            pPdd->ep[mapped_epNum]->pMappedBufPtr = NULL;
            pPdd->ep[mapped_epNum]->bPagesLocked = FALSE;
            pPdd->ep[mapped_epNum]->pTransfer = NULL;
            pTransfer->dwUsbError = UFN_CANCELED_ERROR;

            UNLOCK();

            pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);

            return ERROR_TIMEOUT;  // should be an error code?
        }

        if ((INREG32(&pPdd->pUSBDRegs->OTG.ENDPTSTATUS)&(*temp))==0)
        {
            if ( (INREG32(&pPdd->pUSBDRegs->OTG.ENDPTCOMPLETE)&(*temp))==0 )
            {
                DEBUGMSG(ZONE_ERROR, (L"############# Prime Failed for ep=%d STAT:0x%x WITH:0x%x try again??\r\n",
                    epNum,
                    INREG32(&pPdd->pUSBDRegs->OTG.ENDPTSTATUS),
                    *temp
                    ));
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
                                  VOID *pPddContext, DWORD endPoint, STransfer *pTransfer
                                  )
{
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS *pUSBDRegs = pPdd->pUSBDRegs;


    DWORD epNum, mappedEp;

    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd_AbortTransfer+ (EP= %d)\r\n", endPoint));

    CeLogMsg(_T("AbortTransfer (%s EP%d)"),
        pTransfer ? ( TRANSFER_IS_IN(pTransfer) ? _T("IN") : _T("OUT") ) : _T("NULL"),
        endPoint );

    // Select EP
    epNum = USBD_EP_NUM & endPoint;

    if ( !pTransfer )
    {
        DEBUGMSG(ZONE_ERROR, (L"UsbFnPdd_AbortTransfer: pTransfer is NULL on EP%d!\r\n", epNum));

        return ERROR_INVALID_PARAMETER;
    }

    if ( epNum == 0 )
    {
        mappedEp = TRANSFER_IS_IN(pTransfer) ? 0 : USBD_EP_NUM - 1;
    }
    else
        mappedEp = epNum;

    // make sure the EP currently is running the transfer we're talking about

    LOCK();

    if ( pPdd->ep[mappedEp]->pTransfer == pTransfer )
    {
        FlushEndpoint( pPdd, mappedEp, TRANSFER_IS_OUT(pTransfer) );

        if ( pPdd->ep[mappedEp]->bPagesLocked )
        {
            DEBUGCHK( pPdd->ep[mappedEp]->pMappedBufPtr != NULL );
            UnlockPages(pPdd->ep[mappedEp]->pMappedBufPtr, pTransfer->cbBuffer);
            pPdd->ep[mappedEp]->bPagesLocked = 0;

            CeCloseCallerBuffer(
                (LPVOID)pPdd->ep[mappedEp]->pMappedBufPtr,
                (LPVOID)pTransfer->pvBuffer,
                pTransfer->cbBuffer,
                ARG_IO_PTR
                );
        }

        // Finish transfer
        pPdd->ep[mappedEp]->pTransfer = NULL;

        pTransfer->dwUsbError = UFN_CANCELED_ERROR;
        UNLOCK();

        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);

    }
    else
    {
        DEBUGMSG(ZONE_PDD,(TEXT("Abort(%d) skipped (Tran 0x%x != 0x%x)\r\n"),
            mappedEp,
            pPdd->ep[mappedEp]->pTransfer,
            pTransfer ) );

        pTransfer->dwUsbError = UFN_CANCELED_ERROR;

        UNLOCK();

        pPdd->pfnNotify(pPdd->pMddContext, UFN_MSG_TRANSFER_COMPLETE, (DWORD)pTransfer);
    }


    DEBUGMSG(ZONE_PDD, (L"UsbFnPdd_AbortTransfer- (EP= %d)\r\n", endPoint));


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
//-------------------------------------------------------------------------
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
            edptctrl.TXS=1;
        else
            edptctrl.RXS=1;

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
//-------------------------------------------------------------------------
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
//-------------------------------------------------------------------------

DWORD WINAPI UfnPdd_IsEndpointHalted(
                                     VOID *pPddContext, DWORD endPoint, BOOL *pHalted
                                     )
{
    DWORD rc = ERROR_INVALID_FUNCTION;
    USBFN_PDD *pPdd = pPddContext;
    CSP_USB_REGS *pUSBDRegs = pPdd->pUSBDRegs;
    DWORD epNum;

    // Endpoint can't be zero
    if (endPoint == 0)
        goto clean;

    // Select EP
    epNum = USBD_EP_NUM & endPoint;

    LOCK();
    // Is EP halted?
    {
        USB_ENDPTCTRL_T ctrl;
        DWORD *t=(DWORD*)&ctrl;
        *t=INREG32(&pUSBDRegs->OTG.ENDPTCTRL[epNum-1]);
        if (pPdd->ep[endPoint]->dirRx)
            *pHalted=ctrl.RXS;
        else
            *pHalted =ctrl.TXS;
    }
    UNLOCK();

    // Done
    rc = ERROR_SUCCESS;

clean:
    DEBUGMSG(ZONE_FUNCTION, (L"-UsbFnPdd_IsEndpointHalted %d\r\n", endPoint));
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
//-------------------------------------------------------------------------
DWORD WINAPI UfnPdd_SendControlStatusHandshake(
    VOID *pPddContext, DWORD endPoint
    )
{
    USBFN_PDD *pPdd = pPddContext;

    CeLogMsg(_T("+SendCSH (EP%d)"), endPoint );

    LOCK();
    if ( pPdd->NSend0ACK )
    {
        DEBUGMSG(ZONE_TRANSFER,(_T("SendCSH OUT (%d)\r\n"), endPoint) );

        PrimeQh0ForZeroTransfer(pPdd, FALSE );

        // just prime for zero length OUT (receive data)
        pPdd->NSend0ACK = FALSE;
        pPdd->StatusOutPrimed = TRUE;
    }
    else
    {
        DEBUGMSG(ZONE_TRANSFER,(_T("SendCSH IN (%d)\r\n"), endPoint) );

        PrimeQh0ForZeroTransfer(pPdd, TRUE );
    }

    SetEvent(pPdd->hIntrEvent);  // and trigger interrupt to process this

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
//-------------------------------------------------------------------------

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
//-------------------------------------------------------------------------
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
#ifdef USBFN_TEST_MODE_SUPPORT

    CSP_USB_REGS*pUSBDRegs = pPdd->pUSBDRegs;

    USB_PORTSC_T portsc;
    DWORD  *pTsc;

    pTsc  = (DWORD *)&portsc;

    /* the device controller set select bits are actually the same definitions as in the USB
    * spec for the SetFeature command, but have a switch just in case the controller defines change
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
                              VOID *pPddContext, IOCTL_SOURCE source, DWORD code, UCHAR *pInBuffer,
                              DWORD inSize, UCHAR *pOutBuffer, DWORD outSize, DWORD *pOutSize
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

#ifdef USBFN_TEST_MODE_SUPPORT
    case IOCTL_UFN_SET_TEST_MODE:
        {
            DWORD dwTestMode;

            if (source != MDD_IOCTL)
                break;

            if (pInBuffer == NULL || inSize < sizeof(DWORD))
                break;


            dwTestMode = *(DWORD*)pInBuffer;

            if ( (dwTestMode >= 0) && (dwTestMode <= USB_TEST_MODE_MAX) )
            {
                pPdd->iTestMode = (int)dwTestMode;
                pPdd->bEnterTestMode = TRUE;
                SetEvent(pPdd->hIntrEvent);

                rc = ERROR_SUCCESS;
            }
        }
        break;
#endif

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
        //Function event is also set as deinit can be called when
        //function controller is waiting on a function event from transceiver
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

    USBClockDeleteFileMapping();

    if ( pPdd->qhbuffer )
    {
        FreePhysMem(pPdd->qhbuffer);
        pPdd->qhbuffer = NULL;
    }

    // Free PDD context
    LocalFree(pPdd);
    DEBUGMSG(ZONE_PDD, (_T("UfnPdd_Deinit: -\r\n")) );

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
        UfnPdd_PowerUp(pPdd);
    }

    USBClockDisable(FALSE);
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
//  This function is called when pipe to endpoint is closed. For Freescale iMx31 we
//  will stop points in UfnPdd_DeregisterDevice.
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

    // if this endpoint is isochronous
    if( pPdd->ep[epNum]->bIsochronous )
    {
        DWORD isochSlotIx = GetEpIsochDataIdx(pPdd, pPdd->ep[epNum]) ;
        if( isochSlotIx < USB_MAX_ISOCH_ENDPOINTS )
        {
            pPdd->bIsochEPBusy[ isochSlotIx ] = 0;
        }
        else
        {
            ERRORMSG(TRUE, (TEXT("ERROR: UfnPdd_DeinitEndpoint: ")
                            TEXT("Isoch endpoint has corrupted data structures, epNum=%d, isochSlotIx=%d\r\n"), epNum, isochSlotIx));
        }
        ResetIsochEPData(pPdd->ep[epNum]);
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
                                 VOID *pContext, DWORD endPoint, UFN_BUS_SPEED speed,
                                 USB_ENDPOINT_DESCRIPTOR *pEPDesc, VOID *pReserved, UCHAR configValue,
                                 UCHAR interfaceNumber, UCHAR alternateSetting
                                 )
{

    USBFN_PDD *pPdd = pContext;
    USB_ENDPTCTRL_T edptctrl;
    DWORD epNum, *temp=(DWORD *)&edptctrl;

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
        pPdd->ep[epNum]->bIsochronous = (( (pEPDesc->bmAttributes)&USB_ENDPOINT_TYPE_MASK) == USB_ENDPOINT_TYPE_ISOCHRONOUS);
        ResetIsochEPData(pPdd->ep[epNum]);

        // if this endpoint is isochronous
        if( pPdd->ep[epNum]->bIsochronous )
        {
            DWORD freeIsochIdx = GetFreeIsochDataIdx(pPdd);
            // if we still have room in the pool of isochronous data
            if( freeIsochIdx < USB_MAX_ISOCH_ENDPOINTS  )
            {
                pPdd->ep[epNum]->pIsochData = &(pPdd->qhbuffer->isoch_ep_buffers[freeIsochIdx]) ; // this endpoint takes this chunk of data
                pPdd->bIsochEPBusy[freeIsochIdx] = 1;
                InitIsochEPLinkList( pPdd->ep[epNum] );
            }
            else
            {
                // we ran out of slots for isochronous endpoints... too bad.  we'll continue, but turn off the nice isoch logic.
                pPdd->ep[epNum]->bIsochronous = FALSE;
                DEBUGMSG(ZONE_PDD, (L"Init Endpoint: %x, Ran out of isoch slots\r\n",endPoint ));
            }
        }

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
    if(prevPowerState == D3 || prevPowerState== D4)
    {
        pPdd->m_CurSelfPowerState = D2;
        UpdateDevicePower(pPdd);
    }

    USBClockDisable(FALSE);

    // Initialize the device controller only if B-Type cable is connected to the USB port
    // Or during boot time
    if ((g_isStartUp == TRUE) || (INREG32(&pUSBDRegs->OTG.OTGSC) & 0x100))
    {
        OUTREG32(&pUSBDRegs->OTG.T_158H.ENDPOINTLISTADDR,pPdd->qhbuf_phy);
        // Enable interrupts
        OUTREG32(&pUSBDRegs->OTG.USBINTR, USBD_IRQ_MASK);

        // initial endpoints
        for (i=0;i<15;i++)
            OUTREG32(&pUSBDRegs->OTG.ENDPTCTRL[i], 0x400040);

        OUTREG32(&pUSBDRegs->OTG.T_154H.USBADR, 0);

        // Set the run bit
        SETREG32(&pUSBDRegs->OTG.USBCMD, 1);
        DEBUGMSG(ZONE_PDD, (TEXT("UfnPdd_Start - B Type is connected\r\n")));
    }

    UNLOCK();

    InitQh0(pPdd,0, NULL);
    // Set fake device change flag which on first interrupt force
    // device state change handler even if it isn't indicated by hardware
    pPdd->m_CurSelfPowerState = prevPowerState;
    UpdateDevicePower(pPdd);

    //Transfer the control back to transceiver if cable is not connected
    if((g_isStartUp == TRUE) && (pPdd->bInUSBFN == FALSE))
    {
        g_isStartUp = FALSE;
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
                                   VOID *pPddContext, const USB_DEVICE_DESCRIPTOR *pHighSpeedDeviceDesc,
                                   const UFN_CONFIGURATION *pHighSpeedConfig,
                                   const USB_CONFIGURATION_DESCRIPTOR *pHighSpeedConfigDesc,
                                   const USB_DEVICE_DESCRIPTOR *pFullSpeedDeviceDesc,
                                   const UFN_CONFIGURATION *pFullSpeedConfig,
                                   const USB_CONFIGURATION_DESCRIPTOR *pFullSpeedConfigDesc,
                                   const UFN_STRING_SET *pStringSets, DWORD stringSets
                                   )
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    USBFN_PDD *pPdd = pPddContext;
    UFN_INTERFACE *pIFC;
    UFN_ENDPOINT *pEP;
    DWORD offset, ep;
    DWORD ifc, epx;

    LOCK();

    // Remember self powered flag

    pPdd->selfPowered = (pFullSpeedConfig->Descriptor.bmAttributes & 0x20) != 0;
    pPdd->selfPoweredh = (pHighSpeedConfig->Descriptor.bmAttributes & 0x20) != 0;

    // Configure Full speed EPs
    offset = 8 ;
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
                pPdd->epf[ep].dirRx = FALSE;
            else
                pPdd->epf[ep].dirRx = TRUE;

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
    pPdd->eph[USBD_EP_COUNT-1].maxPacketSize = pHighSpeedDeviceDesc->bMaxPacketSize0;
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
                pPdd->eph[ep].dirRx = FALSE;
            else
                pPdd->eph[ep].dirRx = TRUE;

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
//-----------------------------------------------------------------------
DWORD WINAPI UfnPdd_IsEndpointSupportable(
    VOID *pPddContext, DWORD endPoint, UFN_BUS_SPEED speed,
    USB_ENDPOINT_DESCRIPTOR *pEPDesc, UCHAR configurationValue,
    UCHAR interfaceNumber, UCHAR alternateSetting
    )
{
    USBFN_PDD *pPdd = pPddContext;

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
/// than MAX_SIZE_PER_TD bytes and round EP sizes. Unfortunately we don't get information
//  about EP0 max packet size. So we will assume maximal 64 byte size.
//
// -------------------------------------------------------------------------------
DWORD WINAPI UfnPdd_IsConfigurationSupportable(
    VOID *pPddContext, UFN_BUS_SPEED speed, UFN_CONFIGURATION *pConfig
    )
{
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
    for (ifc = 0; ifc < pConfig->Descriptor.bNumInterfaces; ifc++)
    {
        // For each endpoint in interface
        DEBUGMSG(ZONE_FUNCTION, (TEXT("+UfnPdd_IsConfigurationSupportable ifc 0x%x  \r\n"),ifc));

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
                         LPCTSTR szActiveKey, VOID *pMddContext, UFN_MDD_INTERFACE_INFO *pMddIfc,
                         UFN_PDD_INTERFACE_INFO *pPddIfc
                         )
{
    DWORD rc = ERROR_INVALID_PARAMETER;
    USBFN_PDD *pPdd;
    CSP_USB_REGS *pUSBDRegs;
    //DEVICE_LOCATION devLoc;
    PHYSICAL_ADDRESS pa;
    //DWORD ep;

    //NKDbgPrintfW(TEXT("UfnPdd_Init, build on %s, at %s\r\n"), TEXT(__DATE__), TEXT(__TIME__));

    // Allocate PDD object
    pPdd = LocalAlloc(LPTR, sizeof(USBFN_PDD));
    if (pPdd == NULL)
    {
        DEBUGMSG(ZONE_PDD, (TEXT("UfnPdd_Init pPdd allocate failed\r\n")));
        goto clean;
    }

    memset(pPdd, 0, sizeof(USBFN_PDD));

    USBClockCreateFileMapping();

    InitializeCriticalSection(&g_csRegister);
    InitializeCriticalSection(&g_csIsoc);

    // Initialize critical section
    InitializeCriticalSection(&pPdd->epCS);
    pPdd->devState = 0;
    pPdd->bInUSBFN = TRUE;
    pPdd->bResume = FALSE;
    pPdd->bEnterTestMode = FALSE;
    pPdd->bUSBCoreClk = TRUE;
    pPdd->bUSBPanicMode = TRUE;

    g_isStartUp = TRUE;
    // Read device parameters
    if (GetDeviceRegistryParams(szActiveKey, pPdd, dimof(g_deviceRegParams), g_deviceRegParams) != ERROR_SUCCESS)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: " L"Failed read registry parameters\r\n"));
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
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Controller registers mapping failed\r\n"));
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

    USBClockDisable(FALSE);
    {
        USB_PORTSC_T portsc;
        DWORD *temp = (DWORD *)&portsc;
        *temp = INREG32(&(pPdd->pUSBDRegs)->OTG.PORTSC);
        //Make sure PHY clock is enabled
        if (portsc.PHCD == 1)
        {
            portsc.PHCD = 0;
            OUTREG32(&(pPdd->pUSBDRegs)->OTG.PORTSC, *temp);
            //Delay provided for the PHY clock to be stable
            Sleep(50);
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
        DWORD         *t=(DWORD*)&temp;
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

    pPdd->m_CurSelfPowerState = D4; // Assume Detached First.
    memset( pPdd->bIsochEPBusy, 0, sizeof(pPdd->bIsochEPBusy) );
    UpdateDevicePower(pPdd);

    // Request SYSINTR for interrupts
#if 0
    if (!KernelIoControl(
        IOCTL_HAL_REQUEST_SYSINTR, &(pPdd->irq), sizeof(DWORD),
        &pPdd->sysIntr, sizeof(pPdd->sysIntr), NULL
        )) goto clean;
#else
    pPdd->sysIntr = GetSysIntr();
#endif

    // Create interrupt event
    pPdd->hIntrEvent = CreateEvent(0, FALSE, FALSE, NULL);
    if (pPdd->hIntrEvent == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Error creating interrupt event\r\n"    ));
        goto clean;
    }

    // Initialize interrupt
    if (!InterruptInitialize(pPdd->sysIntr, pPdd->hIntrEvent, NULL, 0))
    {
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Interrupt initialization failed\r\n"));
        goto clean;
    }

    DEBUGMSG(ZONE_INTERRUPTS, (TEXT("USBFNPDD SysIntr:0x%x, Irq:0x%x\r\n"), pPdd->sysIntr, pPdd->irq));
    if (pPdd->IsOTGSupport)
    {
        InterruptDisable(pPdd->sysIntr);
    }
    else
    {
        // Enable Kernel WakeUp
        KernelIoControl(IOCTL_HAL_ENABLE_WAKE, &pPdd->sysIntr, sizeof(pPdd->sysIntr), NULL, 0, NULL);
    }

    // Set PDD interface
    pPddIfc->dwVersion = UFN_PDD_INTERFACE_VERSION;
    pPddIfc->dwCapabilities = UFN_PDD_CAPS_SUPPORTS_FULL_SPEED| UFN_PDD_CAPS_SUPPORTS_HIGH_SPEED| UFN_PDD_CAPS_SUPPORTS_MULTIPLE_CONFIGURATIONS;
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
        DEBUGMSG(ZONE_ERROR, (L"ERROR: UfnPdd_Init: Interrupt thread creation failed\r\n"));
        goto clean;
    }

    CeSetThreadPriority( pPdd->hIntrThread, pPdd->priority256 ) ;

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
    return TRUE;
}


//------------------------------------------------------------------------------
