//-----------------------------------------------------------------------------
//
//  Copyright (C) 2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ddi_dcp_isr_win32.c
//  Implement data co-processor interrupt process depending on OS
//
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115)
#include <windows.h>
#pragma warning(pop)

#include "ddi_dcp_os_private.h"
#pragma warning(disable: 4702)

extern PVOID    pv_HWregDCP;

extern DCPChannel_t* g_DCPChannel;

static HANDLE dcp_thread;
static HANDLE dcp_isr_event;
static DWORD SysIntrDCP;

RtResult dcp_MakeChannelAvailable(UINT32 Channel);

DWORD CSPDCPGetIRQ();
void DCPCSCInterruptHandler();

DWORD WINAPI ddi_dcp_isr_thread(LPVOID lpParameter);
RtResult ddi_dcp_ServiceChannelIRQ(UINT32 Channel);

//-----------------------------------------------------------------------------
//
// Function : dcp_isr_Init
//
//  Initialize DCP ISR routine
//
// Parameters:
//      None
//
// Returns:
//      None
//
//-----------------------------------------------------------------------------
void dcp_isr_Init(void)
{
    DWORD irq = CSPDCPGetIRQ();
    dcp_isr_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (dcp_isr_event == NULL) {
        ERRORMSG(1, (TEXT("Unable to create DCP ISR event!\r\n")));
        return;
    }//if

    if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &irq, sizeof(DWORD),
        &SysIntrDCP, sizeof(DWORD), NULL))
    {
        ERRORMSG(1, (TEXT("ERROR: Failed to obtain sysintr value for PXP interrupt.\r\n")));
        return;
    }

    if ( !InterruptInitialize(SysIntrDCP,
        dcp_isr_event,
        NULL,
        0) ) {
        ERRORMSG(1, (TEXT("Unable to Initialize DCP ISR!\r\n")));
        return;
    }
    InterruptDone(SysIntrDCP);

    // CreateThread for Screen Refresh
    dcp_thread = CreateThread(NULL, 0, ddi_dcp_isr_thread, NULL, 0, NULL);
    if (dcp_thread == NULL) {
        ERRORMSG(1, (TEXT("Unable to create DCP thread!\r\n")));
        return;
    }//if

}

//-----------------------------------------------------------------------------
//
// Function : ddi_dcp_isr_thread
//
// This method is a thread to handle DCP interrupt
//
// Parameters:
//      lpParameter
//          [in] The thread data passed to the function using the lpParameter parameter.
//
// Returns:
//      The thread return value
//
//-----------------------------------------------------------------------------
DWORD WINAPI ddi_dcp_isr_thread(LPVOID lpParameter)
{
    UINT32      i;
    UINT32      Channel;
    UNREFERENCED_PARAMETER(lpParameter);

    for (;;) {
        WaitForSingleObject(dcp_isr_event, INFINITE);

        Channel = HW_DCP_STAT_RD() & 0x10f;

        if (Channel & 0x100) {
            DCPCSCInterruptHandler();
        } else {
            for (i = 0; i < DCP_MAX_CHANNELS; i++) {
                if (Channel & (1 << i)) {
                    ddi_dcp_ServiceChannelIRQ(i);
                    break;
                }//if
            }//for
        }//if/else

        InterruptDone(SysIntrDCP);
    }//for

    return 0;
}

//-----------------------------------------------------------------------------
//
// Function : ddi_dcp_ServiceChannelIRQ
//
// This function processes DCP channel interrupt
//
// Parameters:
//      Channel
//          [in] DCP channel number
//
// Returns:
//      ERROR_SUCCESS
//
//-----------------------------------------------------------------------------
RtResult ddi_dcp_ServiceChannelIRQ(UINT32 Channel)
{
    dcp_hw_ChannelInterruptClear(Channel);

    if (g_DCPChannel[Channel].CallbackFunction != NULL)
    {
        (g_DCPChannel[Channel].CallbackFunction)(g_DCPChannel[Channel].CallbackData);
    }

    if (g_DCPChannel[Channel].Locked == 0)
    {
        dcp_MakeChannelAvailable(Channel);
    }

    dcp_PutChannelSemaphore(&g_DCPChannel[Channel]);

    return ERROR_SUCCESS;
}

