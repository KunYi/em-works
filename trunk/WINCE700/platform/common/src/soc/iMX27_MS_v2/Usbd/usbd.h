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
//  File:  usbd.h
//
//  The header file for USB function driver.
//
//-----------------------------------------------------------------------------
#ifndef USBD_H
#define USBD_H

#define USBD_EP_COUNT   16
#define USBD_PORT_NUM    0


#define USB_MAX_ISOCH_ENDPOINTS 4
#define USB_ISOCH_TD_ENTRIES  20  // TD entries used for isoch transfers




typedef struct _USBD_dTD
{
    unsigned int T:1;        // Terminate (T).
    // 1=pointer is invalid. 0=Pointer is valid (points to a valid Transfer Element Descriptor). This bit indicates to the Device Controller that there are no more valid entries in the queue.
    unsigned int r4:4;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int next_dtd:27;// Next Transfer Element Pointer.
    // This field contains the physical memory address of the next dTD to be processed. The field corresponds to memory address signals [31:5], respectively.

    unsigned int status:8;   // This field is used by the Device Controller to communicate individual command execution states back to the Device Controller software. This field
    // contains the status of the last transaction performed on this qTD. The bit encodings are:
    // 7 Active.
    // 6 Halted.
    // 5 Data Buffer Error.
    // 3 Transaction Error.
    // 4,2,0Reserved.
    unsigned int r5:2;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int MultO:2;    // Multiplier Override (MultO). This field can be used for transmit ISO's (ie.ISO-IN) to override the multiplier in the QH. This field must be zero for all packet types that are not transmit-ISO.
    // Example:
    // if QH.multiplier = 3; Maximum packet size = 8; Total Bytes = 15; MultiO = 0
    // [default]
    // Three packets are sent: {Data2(8); Data1(7); Data0(0)}
    // if QH.multiplier = 3; Maximum packet size = 8; Total Bytes = 15; MultiO = 2
    // Two packets are sent: {Data1(8); Data0(7)}
    // For maximal efficiency, software should compute MultO = greatest integer of
    // (Total Bytes / Max. Packet Size) except for the case when Total Bytes = 0; then
    // MultO should be 1.
    // Note: Non-ISO and Non-TX endpoints must set MultO=§00§.
    unsigned int r6:3;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int ioc:1;      // Interrupt On Complete (IOC). This bit is used to indicate if USBINT is to be set in response to device controller being finished with this dTD.
    unsigned int tb:15;      // Total Bytes.
    // This field specifies the total number of bytes to be moved with this transfer descriptor. This field is decremented by the number of bytes actually
    // moved during the transaction and only on the successful completion of the transaction.
    // The maximum value software may store in the field is 5*4K(5000H). This is the maximum number of bytes 5 page pointers can access. Although it is possible to
    // create a transfer up to 20K this assumes the 1st offset into the first page is 0. When the offset cannot be predetermined, crossing past the 5th page can be
    // guaranteed by limiting the total bytes to 16K**. Therefore, the maximum recommended transfer is 16K(4000H).
    // If the value of the field is zero when the host controller fetches this transfer descriptor (and the active bit is set), the device controller executes a zero-length
    // transaction and retires the transfer descriptor.
    // It is not a requirement for IN transfers that Total Bytes To Transfer be an even multiple of Maximum Packet Length. If software builds such a transfer descriptor
    // for an IN transfer, the last transaction will always be less that Maximum Packet Length.
    unsigned int r7:1;       // Reserved. Bits reserved for future use and should be set to zero.

    unsigned int curr_off:12;// Current Offset. Offset into the 4kb buffer where the packet is to begin.
    unsigned int bp0:20;     // Buffer Pointer.
    // Selects the page offset in memory for the packet buffer. Non virtual memory systems will typically set the buffer pointers to a series of incrementing integers.

    unsigned int fn:11;      // Frame Number.
    // Written by the device controller to indicate the frame number in which a packet finishes. This is typically be used to correlate relative completion times of packets on an ISO endpoint.
    unsigned int r8:1;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int bp1:20;

    unsigned int r9:12;      // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int bp2:20;

    unsigned int r10:12;     // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int bp3:20;

    unsigned int r11:12;     // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int bp4:20;

} USBD_dTD_T, *PUSBD_dTD_T;

typedef struct _USBD_dTD_R
{
    unsigned int T:1;        // Terminate (T).
    // 1=pointer is invalid. 0=Pointer is valid (points to a valid Transfer Element Descriptor). This bit indicates to the Device Controller that there are no more valid entries in the queue.
    unsigned int r4:4;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int next_dtd:27;// Next Transfer Element Pointer.
    // This field contains the physical memory address of the next dTD to be processed. The field corresponds to memory address signals [31:5], respectively.

    unsigned int status:8;   // This field is used by the Device Controller to communicate individual command execution states back to the Device Controller software. This field
    // contains the status of the last transaction performed on this qTD. The bit encodings are:
    // 7 Active.
    // 6 Halted.
    // 5 Data Buffer Error.
    // 3 Transaction Error.
    // 4,2,0Reserved.
    unsigned int r5:2;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int MultO:2;    // Multiplier Override (MultO). This field can be used for transmit ISO's (ie.ISO-IN) to override the multiplier in the QH. This field must be zero for all packet types that are not transmit-ISO.
    // Example:
    // if QH.multiplier = 3; Maximum packet size = 8; Total Bytes = 15; MultiO = 0
    // [default]
    // Three packets are sent: {Data2(8); Data1(7); Data0(0)}
    // if QH.multiplier = 3; Maximum packet size = 8; Total Bytes = 15; MultiO = 2
    // Two packets are sent: {Data1(8); Data0(7)}
    // For maximal efficiency, software should compute MultO = greatest integer of
    // (Total Bytes / Max. Packet Size) except for the case when Total Bytes = 0; then
    // MultO should be 1.
    // Note: Non-ISO and Non-TX endpoints must set MultO=§00§.
    unsigned int r6:3;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int ioc:1;      // Interrupt On Complete (IOC). This bit is used to indicate if USBINT is to be set in response to device controller being finished with this dTD.
    unsigned int tb:15;      // Total Bytes.
    // This field specifies the total number of bytes to be moved with this transfer descriptor. This field is decremented by the number of bytes actually
    // moved during the transaction and only on the successful completion of the transaction.
    // The maximum value software may store in the field is 5*4K(5000H). This is the maximum number of bytes 5 page pointers can access. Although it is possible to
    // create a transfer up to 20K this assumes the 1st offset into the first page is 0. When the offset cannot be predetermined, crossing past the 5th page can be
    // guaranteed by limiting the total bytes to 16K**. Therefore, the maximum recommended transfer is 16K(4000H).
    // If the value of the field is zero when the host controller fetches this transfer descriptor (and the active bit is set), the device controller executes a zero-length
    // transaction and retires the transfer descriptor.
    // It is not a requirement for IN transfers that Total Bytes To Transfer be an even multiple of Maximum Packet Length. If software builds such a transfer descriptor
    // for an IN transfer, the last transaction will always be less that Maximum Packet Length.
    unsigned int r7:1;       // Reserved. Bits reserved for future use and should be set to zero.

    unsigned int curr_off:12;// Current Offset. Offset into the 4kb buffer where the packet is to begin.
    unsigned int bp0:20;     // Buffer Pointer.
    // Selects the page offset in memory for the packet buffer. Non virtual memory systems will typically set the buffer pointers to a series of incrementing integers.

    unsigned int fn:11;      // Frame Number.
    // Written by the device controller to indicate the frame number in which a packet finishes. This is typically be used to correlate relative completion times of packets on an ISO endpoint.
    unsigned int r8:1;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int bp1:20;

    unsigned int r9:12;      // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int bp2:20;

    unsigned int r10:12;     // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int bp3:20;

    unsigned int r11:12;     // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int bp4:20;
    unsigned int r12[9];     // Reserved. Bits reserved for future use and should be set to zero.
} USBD_dTD_R_T, *PUSBD_dTD_R_T;

typedef struct _USBD_dQH
{
    unsigned int r1:15;      // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int ios:1;      // Interrupt On Setup (IOS). This bit is used on control type endpoints to indicate if USBINT is set in response to a setup being received.
    unsigned int mpl:11;     // Maximum Packet Length.
    // This directly corresponds to the maximum packet size of the associated endpoint (wMaxPacketSize). The maximum value this field may contain is 0x400 (1024).
    unsigned int r2:2;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int zlt:1;      // Zero Length Termination Select.
    // This bit is used to indicate when a zero length packet is used to terminate transfers where to total transfer length is a multiple .
    // This bit is not relevant for Isochronous
    // 0 每 Enable zero length packet to terminate transfers equal to a multiple of the Maximum Packet Length. (default).
    // 1 每 Disable the zero length packet on transfers that are equal in length to a multiple Maximum Packet Length.
    unsigned int mult:2;     // This field is used to indicate the number of packets executed per transaction
    // 00 每 Execute N Transactions as demonstrated by the USB variable length packet protocol where N is computed using the Maximum Packet Length (dQH) and the    Total Bytes field (dTD)
    // 01 每 Execute 1 Transaction.
    // 10 每 Execute 2 Transactions.
    // 11 每 Execute 3 Transactions.
    // Note: Non-ISO endpoints must set Mult=0
    // Note: ISO endpoints must set Mult= 00, 01 or 11 as needed.

    unsigned int r3:5;       // Reserved. Bits reserved for future use and should be set to zero.
    unsigned int curr_dTD:27;// Current dTD.
    // This field is a pointer to the dTD that is represented in the transfer overlay area. This field will be modified by the Device Controller to next dTD pointer during endpoint priming or queue advance.

    USBD_dTD_T  dtd;         // Overlay dTD    area

    unsigned int r4;         // Reserved. Bits reserved for future use and should be set to zero. for 64 bytes alignment

    //The set-up buffer is dedicated storage for the 8-byte data that follows a set-up PID.
    unsigned int SB0:32;     // Setup Buffer 0.
    // This buffer contains bytes 3 to 0 of an incoming setup buffer packet and is written by the device controller to be read by software.
    unsigned int SB1:32;     // Setup Buffer 1.
    // This buffer contains bytes 7 to 4 of an incoming setup buffer packet and is written by the device controller to be read by software.
    unsigned int r5[4];      // Reserved. Bits reserved for future use and should be set to zero. for 64 bytes alignment
} USBD_dQH_T, *PUSBD_dQH_T;


//------------------------------------------------------------------------------
//
//  Define:  USBD_IRQ_MASK
//
//  This is composite interrupt mask used in driver.
//  UE | UEE | PCE |URE |SLE | ULPI
#define USBD_IRQ_MASK 0x157

#define USBD_EP_NUM                 0x000F          // EP number

#define MAX_PACKET_SIZE    4096
//#define MAX_PACKET_SIZE    8192

#define MAX_SIZE_PER_BP   4096
#define MAX_SIZE_PER_TD   (16 * 1024)

#define USB_MAX_TRANSFER   (256 * 1024)   // 256 k transfers at most
#define USB_MAX_DESC_COUNT ((USB_MAX_TRANSFER / 4096)+1)



//------------------------------------------------------------------------------
//
//  Type:  USBFN_ISOCH_EP_DATA
//
//  Internal PDD structure which holds info about isochronous endpoints.
//

__declspec(align(32)) typedef struct
{
    // VERY IMPORTANT NOTE: this struct must be in memory in an address that
    // is a multiple of 32.  So if we change this struct in any way,
    // padding has to be added to make the sizeof() be a multiple of 32!
    USBD_dTD_R_T  isochtd[USB_ISOCH_TD_ENTRIES];
} USBFN_ISOCH_EP_DATA;


//------------------------------------------------------------------------------
//
//  Type:  USBFN_EP
//
//  Internal PDD structure which holds info about endpoint direction,
//  max packet size and active transfer.
//
typedef struct
{
    WORD maxPacketSize;
    BOOL dirRx;
    BOOL fZeroLengthNeeded;

    STransfer *pTransfer;

    BOOL  bPagesLocked;
    LPBYTE pMappedBufPtr;
    // these are not used for isochronous endpoints
    DWORD pdwPageList[USB_MAX_DESC_COUNT];  // list of physical pages for transfer buffer
    DWORD dwNextPageIdx;                    // index of next physical page to be used in TD
    DWORD dwNumPages;

    BOOL  bIsochronous;

    // the data below is only kept for isochronous endpoints
    USBFN_ISOCH_EP_DATA *pIsochData; // only valid for isochronous endpoints - NULL otherwise

    // pointers to CIRCULAR list of buffers.  the list starts at pIsochData, and its size is USB_ISOCH_TD_ENTRIES
    USBD_dTD_R_T *pFirstUsedTd;
    USBD_dTD_R_T *pFirstFreeTd; // points JUST PAST the last used TD (and if it points to the first element, then the last one used is the last one of the list)
    DWORD dwNumTdsUsed; // this many TDs are in possession of the hardware.

} USBFN_EP;


typedef struct _USBFN_QH_BUF
{
    USBD_dQH_T qh[2*USBD_EP_COUNT];
    USBD_dTD_R_T  td[USBD_EP_COUNT*2];
    USBFN_ISOCH_EP_DATA isoch_ep_buffers[USB_MAX_ISOCH_ENDPOINTS];
    BOOL bPrimed[USBD_EP_COUNT];  // indicate if it has been primed before CheckEventLost,
    // Endpoint 0 IN is used by USBD_EP_COUNT-1
    // Endpoint 0 OUT is used by 0

    // Rest of endpoint have no in and out.
} USBFN_QH_BUF_T;

//------------------------------------------------------------------------------
//
//  Type:  USBFN_PDD
//
//  Internal PDD context structure.
//
typedef struct _USBFN_PDD
{
    DWORD memBase;
    DWORD memLen;
    DWORD priority256;
    DWORD irq;

    BYTE bIsochEPBusy[USB_MAX_ISOCH_ENDPOINTS];
    VOID *pMddContext;
    PFN_UFN_MDD_NOTIFY pfnNotify;

    HANDLE hParentBus;
    CSP_USB_REGS*pUSBDRegs;

    DWORD sysIntr;
    HANDLE hIntrEvent;
    BOOL exitIntrThread;
    HANDLE hIntrThread;
    HANDLE hFunctionEvent;
    HANDLE hXcvrEvent;
    HANDLE hTransferEvent;

    CEDEVICE_POWER_STATE    m_CurPMPowerState;
    CEDEVICE_POWER_STATE    m_CurSelfPowerState;
    CEDEVICE_POWER_STATE    m_CurActualPowerState;


    DWORD devState;
    BOOL selfPowered;
    BOOL selfPoweredh;

    CRITICAL_SECTION epCS;
    USBFN_EP *ep[USBD_EP_COUNT];
    USBFN_EP epf[USBD_EP_COUNT];
    USBFN_EP eph[USBD_EP_COUNT];
    BOOL highspeed;
    UCHAR  addr;

    USBFN_QH_BUF_T * qhbuffer;
    DWORD         qhbuf_phy;
    BOOL fakeDsChange;                  // To workaround MDD problem
    BOOL NSend0ACK;

    // Add for USB OTG Support
    BOOL IsOTGSupport;
    TCHAR szOTGGroup[15];
    BOOL bInUSBFN;
    BOOL bResume;

    DWORD dwUSBIntrValue;  // USBINTR value to be saved before power down
    BOOL  bUSBCoreClk;     // TRUE - Core Clock running, FALSE - Stop running
    BOOL  bUSBPanicMode;   // TRUE - Panic Mode (1.65V), FALSE - Non-panic mode
    BOOL  bUSBPanicIntrMask; // Panic Mode Interrupt Mask
    BOOL  bIsMX31TO2; // TRUE- If hardware is TO2 else FALSE

    BOOL bEnterTestMode;
    int  iTestMode;
    BOOL StatusOutPrimed;

} USBFN_PDD;

static enum ISOListFlags{
    ISO_TD_LIST_TRAVERSE_COMPLETE,
    ISO_TD_LIST_TRAVERSE_INCOMPLETE,
    ISO_TD_LIST_MUST_REPRIME
};

extern BOOL HardwareInitialize(CSP_USB_REGS * regs);
extern BOOL HardwarePullupDP(CSP_USB_REGS * regs);
extern DWORD GetSysIntr(void);

#define MAX_RCV_TOTAL_TD 32
#define MAX_RCV_TD 30
#endif
