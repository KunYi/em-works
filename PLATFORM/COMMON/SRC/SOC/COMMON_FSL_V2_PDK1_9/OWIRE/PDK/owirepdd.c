//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  owirepdd.c
//
//  Implementation of One-Wire Driver Platform Device Driver
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4127 4201 4214)
#include <windows.h>
#include <nkintr.h>
#include <ceddk.h>
#pragma warning(pop)

#include "common_macros.h"
#include "common_owire.h"
#include "owire.h"
#include "owire_priv.h"


//-----------------------------------------------------------------------------
// External Functions
extern UINT32 OwireGetBaseRegAddr(void);
extern UINT32 OwireGetIRQ(void);
extern UINT16 BSPGetDivider(void);
extern void BSPOwireIomuxConfig(void);


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables
static PCSP_OWIRE_REGS g_pOWIRE;
static CRITICAL_SECTION g_hOwireLock;
static DWORD g_dwOwireSysIntr;
static HANDLE g_hOwireInt;
static HANDLE g_hOwireRPP;
static HANDLE g_hOwireTSRE;
static HANDLE g_hOwireRBF;
static HANDLE g_hOwireIST;
static BOOL g_bTerminate = FALSE;


//------------------------------------------------------------------------------
// Local Functions
VOID OwireSoftReset(void);
BOOL OwireWriteByte(UINT8);
BOOL OwireReadByte(BYTE*);
BOOL OwireWriteBit(UINT8);
BYTE OwireReadBit();
static DWORD WINAPI OwireIST(LPVOID lpParam);


//------------------------------------------------------------------------------
//
// Function: OwireDeinit
//
// Frees up the register space and deletes critical
// section for deinitialization.
//
// Parameters:
//      None
//
// Returns:
//      None
//
//------------------------------------------------------------------------------
void OwireDeinit(void)
{
    OWIRE_FUNCTION_ENTRY();

    InterruptDisable(g_dwOwireSysIntr);

    // Release the interrupt
    if (!KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &g_dwOwireSysIntr, sizeof(DWORD), NULL, 0, NULL))
        DEBUGMSG(ZONE_ERROR, (TEXT("ERROR: Failed to release dwSysIntr.\r\n")));

    // Kill interrupt service thread
    if (g_hOwireIST)
    {
        g_bTerminate = TRUE;
        PulseEvent(g_hOwireInt);
        CloseHandle(g_hOwireIST);
        g_hOwireIST = NULL;
    }

    // Release Interrupt Occurrence Event -
    if (g_hOwireInt)
    {
        CloseHandle(g_hOwireInt);
        g_hOwireInt = NULL;
    }

    // Release other Events
    if (g_hOwireRBF)
    {
        CloseHandle(g_hOwireRBF);
        g_hOwireRBF = NULL;
    }
    if (g_hOwireRPP)
    {
        CloseHandle(g_hOwireRPP);
        g_hOwireRPP = NULL;
    }
    if (g_hOwireTSRE)
    {
        CloseHandle(g_hOwireTSRE);
        g_hOwireTSRE = NULL;
    }
    
    // Delete the critical section
    DeleteCriticalSection(&g_hOwireLock);

    // Release Register Base Mapped Virtual Memory -
    if (g_pOWIRE!= NULL)
    {
        MmUnmapIoSpace((LPVOID) g_pOWIRE, sizeof(CSP_OWIRE_REGS));
        g_pOWIRE = NULL;
    }

    OWIRE_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: OwireInitialize
//
// This function is the initialization routine for the one-wire.  It
// allocates memory for the one-wire registers and sets the
// time divider register.  If there is a failure in any of the
// initialization steps, OwireRelease will be called to clean up.
//
// Parameters:
//      None
//
// Returns:
//      TRUE - If success
//
//      FALSE - If failure
//
//------------------------------------------------------------------------------
BOOL OwireInitialize(void)
{
    PHYSICAL_ADDRESS phyAddr;
    UINT16 dividerVal;

    OWIRE_FUNCTION_ENTRY();

    phyAddr.QuadPart = OwireGetBaseRegAddr();

    g_pOWIRE = (PCSP_OWIRE_REGS) MmMapIoSpace(phyAddr, sizeof(CSP_OWIRE_REGS), FALSE);

    // check if Map Virtual Address failed
    if (g_pOWIRE == NULL)
    {
        DEBUGMSG(ZONE_ERROR, (TEXT("OwireInitialize:  MmMapIoSpace failed!\r\n")));
        goto Error;
    }
    BSPOwireIomuxConfig();

    // Initialize OWIRE critical section
    InitializeCriticalSection(&g_hOwireLock);

    OwireSoftReset();
    dividerVal = BSPGetDivider();

    // set time divider value to get as close as possible to a 1MHz clock
    OUTREG16(&g_pOWIRE->TD, dividerVal);

    // Create Hardware Interrupt Occurrence Event
    g_hOwireInt= CreateEvent(NULL, FALSE, FALSE, NULL);
    // Able to create or obtain the event?
    if (g_hOwireInt == NULL)
    {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("OwireInitialize:CreateEvent():"),
            TEXT(" Interrupt Occurrence Event Failed! ErrCode=%d \r\n"), GetLastError()));
        goto Error;
    }
    
    // Create Reset Precense Pulse Event
    g_hOwireRPP= CreateEvent(NULL, FALSE, FALSE, NULL);
    // Able to create or obtain the event?
    if (g_hOwireRPP == NULL)
    {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("OwireInitialize:CreateEvent():"),
            TEXT("Reset Presence Pulse Event Failed! ErrCode=%d \r\n"), GetLastError()));
        goto Error;
    }

    // Create Transmit Shift Register Empty Event
    g_hOwireTSRE= CreateEvent(NULL, FALSE, FALSE, NULL);
    // Able to create or obtain the event?
    if (g_hOwireTSRE== NULL)
    {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("OwireInitialize:CreateEvent():"),
            TEXT(" Transmit Shift Register Empty Event Failed! ErrCode=%d \r\n"), GetLastError()));
        goto Error;
    }
    
    // Create Receive Buffer Full Event
    g_hOwireRBF= CreateEvent(NULL, FALSE, FALSE, NULL);
    // Able to create or obtain the event?
    if (g_hOwireRBF == NULL)
    {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("OwireInitialize:CreateEvent():"),
            TEXT(" Receive Buffer Full Event Failed! ErrCode=%d \r\n"), GetLastError()));
        goto Error;
    }

    // Map IRQ -> System Interrupt ID
    {
        // Get OWIRE IRQ Number
        DWORD dwIrq = OwireGetIRQ();

        // Get kernel to translate IRQ -> System Interrupt ID
        if (!KernelIoControl(IOCTL_HAL_REQUEST_SYSINTR, &dwIrq, sizeof(DWORD), &g_dwOwireSysIntr, sizeof(DWORD), NULL))
        {
            DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("OwireInitialize:KernelIoControl():"),
                TEXT(" IRQ -> SysIntr Failed! ErrCode=%d \r\n"), GetLastError()));
            goto Error;
        }
    }

    // Link hInterrupted -> OWIRE Interrupt Pin
    if (!InterruptInitialize(g_dwOwireSysIntr, g_hOwireInt, NULL, 0))
    {
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("OwireInitialize:Interruptinitialize():"),
            TEXT(" Linking failed! ErrCode=%d \r\n"), GetLastError()));
        goto Error;
    }

    // Create IST thread to receive hardware interrupts
    // Start Owire IST thread
    // Initialize thread for Owire interrupt handling
    //      pThreadAttributes = NULL (must be NULL)
    //      dwStackSize = 0 => default stack size determined by linker
    //      lpStartAddress = OwireIST => thread entry point
    //      lpParameter = this => point to thread parameter
    //      dwCreationFlags = 0 => no flags
    //      lpThreadId = NULL => thread ID is not returned
    g_hOwireIST = CreateThread(NULL, 0, OwireIST, NULL, 0, NULL);//NULL);

    if (g_hOwireIST == NULL)
    {
        DEBUGMSG(ZONE_INIT,(TEXT("%s: CreateThread failed!\r\n"), __WFUNCTION__));
        goto Error;
    }
    else
    {
        DEBUGMSG(ZONE_INIT, (TEXT("%s: create OWIRE IST thread success\r\n"), __WFUNCTION__));
        CeSetThreadPriority(g_hOwireIST, 100);//THREAD_PRIORITY_TIME_CRITICAL);
    }

    OWIRE_FUNCTION_EXIT();
    return TRUE;

Error:
    //Cleaning up
    OwireDeinit();
    return FALSE;
}


//------------------------------------------------------------------------------
//
// Function: OwireSoftReset
//
// Software resets the OneWire module.  First set the RESET register
// to 1 and then set it to 0.
//
// Parameters:
//      None.
//
// Returns:
//      None.
//
//------------------------------------------------------------------------------
void OwireSoftReset(void)
{
    OWIRE_FUNCTION_ENTRY();

    // We must manually assert reset and then deassert.
    OUTREG16(&g_pOWIRE->RST, CSP_BITFVAL(OWIRE_RST_RST, OWIRE_RST_RST_RESET));
    // Pull down the bus for at least 480 us
    Sleep(1);
    OUTREG16(&g_pOWIRE->RST, CSP_BITFVAL(OWIRE_RST_RST, OWIRE_RST_RST_ENDRESET));

    OWIRE_FUNCTION_EXIT();
}


//------------------------------------------------------------------------------
//
// Function: OwireSetResetPresencePulse
//
// This function is used to detect the presence of owire device.
// This function should always be called before any
// write or read sequence.
//
// Parameters:
//      None.
//
// Returns:
//      TRUE if owire device exists, presence detected.
//      FALSE if Detection sequence failed
//
//------------------------------------------------------------------------------
BOOL OwireSetResetPresencePulse(void)
{
    DWORD dwEventRet;
    BOOL bRet = FALSE;

    OWIRE_FUNCTION_ENTRY();

    OUTREG16(&g_pOWIRE->IER, OWIRE_EPD_INT_EN);
    INSREG16(&g_pOWIRE->CR, CSP_BITFMASK(OWIRE_CR_RPP), 
        CSP_BITFVAL(OWIRE_CR_RPP, OWIRE_CR_RPP_RESET));

    dwEventRet = WaitForSingleObject(g_hOwireRPP, OWIRE_TIMEOUT);
    if( WAIT_OBJECT_0 == dwEventRet)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("OWIRE: Precense Detected!")));
        bRet = TRUE;
    } else if (WAIT_TIMEOUT == dwEventRet)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("OWIRE: Precense Detect Timeout!!!")));
        bRet = FALSE;
    } else
    {
        DEBUGMSG(ZONE_INFO, (TEXT("OWIRE: Reset / Precense ERROR!!!")));
        bRet = FALSE;
    }
    
    OUTREG16(&g_pOWIRE->IER, OWIRE_INT_CLEAR);
    OWIRE_FUNCTION_EXIT();

    return bRet;
}


//------------------------------------------------------------------------------
//
// Function: OwireWriteData
//
// Write count bytes of data to owire device.
// This function calls OwireWriteByte to write one byte at
// a time from writeBuf.
//
// Parameters:
//      writeBuf
//          [in] Buffer containing bytes
//      count
//          [in] Number of bytes to write from writeBuf
//
// Returns:
//      TRUE if write sequence successful
//      FALSE if write sequence failed
//
//------------------------------------------------------------------------------
BOOL OwireWriteData(BYTE* writeBuf, DWORD count)
{
    UINT32 i;

    OWIRE_FUNCTION_ENTRY();

    OUTREG16(&g_pOWIRE->IER, OWIRE_ETSE_INT_EN);
    // Write one byte at a time
    for (i = 0; i < count; i++) 
    {
        if (!OwireWriteByte(writeBuf[i]))
        {
            OUTREG16(&g_pOWIRE->IER, OWIRE_INT_CLEAR);
            return FALSE;
        }
    }

    OUTREG16(&g_pOWIRE->IER, OWIRE_INT_CLEAR);
    OWIRE_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: OwireReadData
//
// Read count number of bytes from owire device.
// This function calls OwireReadByte to read one byte at
// a time into readBuf.
//
// Parameters:
//      readBuf
//          [out] Buffer for bytes read from owire device
//      count
//          [in] Number of bytes to read into readBuf
//
// Returns:
//      TRUE if read sequence successful
//      FALSE if read sequence failed
//
//------------------------------------------------------------------------------
BOOL OwireReadData(BYTE* readBuf, DWORD count)
{
    UINT32 i;

    OWIRE_FUNCTION_ENTRY();

    i = (UINT32)INREG16(&g_pOWIRE->TRX);
    OUTREG16(&g_pOWIRE->IER, OWIRE_ERBF_INT_EN);
    // Read one byte at a time from the owire device
    for (i = 0; i < count; i++) 
    {
        if (!OwireReadByte(&readBuf[i]))
        {
            OUTREG16(&g_pOWIRE->IER, OWIRE_INT_CLEAR);
            return FALSE;
        }
        DEBUGMSG(ZONE_READWRITE, (TEXT("Read byte[%d] = %x\r\n"), i, readBuf[i]));
    }

    OUTREG16(&g_pOWIRE->IER, OWIRE_INT_CLEAR);
    OWIRE_FUNCTION_EXIT();

    return TRUE;
}


//------------------------------------------------------------------------------
//
// Function: OwireWriteByte
//
// Sends 8 bits of data to owire device.
// This function calls WriteBit to send the 8-bit data one bit at a time.
//
// Parameters:
//      UINT8 byte - 8-bit data to be sent to owire device
//
// Returns:
//      FALSE if writing sequence failed
//      TRUE if writing sequence successful
//
//------------------------------------------------------------------------------
BOOL OwireWriteByte(UINT8 byte)
{
    BOOL bRet = FALSE;
    DWORD dwEventRet;

    OWIRE_FUNCTION_ENTRY();

    DEBUGMSG(ZONE_READWRITE, (TEXT("Writing byte %x\r\n"), byte));
    
    OUTREG16(&g_pOWIRE->TRX, (USHORT)byte);

    dwEventRet = WaitForSingleObject(g_hOwireTSRE, OWIRE_TIMEOUT);
    if( WAIT_OBJECT_0 == dwEventRet)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("OWIRE: Write Byte Done!")));
        bRet = TRUE;
    } else if (WAIT_TIMEOUT == dwEventRet)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("OWIRE: Write Byte Timeout!!!")));
        bRet = FALSE;
    } else
    {
        DEBUGMSG(ZONE_INFO, (TEXT("OWIRE: Write Byte ERROR!!!")));
        bRet = FALSE;
    }
    OWIRE_FUNCTION_EXIT();

    return bRet;
}


//------------------------------------------------------------------------------
//
// Function: OwireReadByte
//
// Read 8 bits of data from owire device.
// This function calls ReadBit to read one bit at
// a time and reconstruct the byte of information.
//
// Parameters:
//      by
//          [out] Pointer to byte where 8-bit data
//          from the owire device is stored
//
// Returns:
//      TRUE if read sequence successful
//      FALSE if read sequence failed
//
//------------------------------------------------------------------------------
BOOL OwireReadByte(UINT8*byte)
{
    BOOL bRet = FALSE;
    DWORD dwEventRet;

    OWIRE_FUNCTION_ENTRY();
    
    OUTREG16(&g_pOWIRE->TRX, OWIRE_READ);

    dwEventRet = WaitForSingleObject(g_hOwireRBF, OWIRE_TIMEOUT);
    if( WAIT_OBJECT_0 == dwEventRet)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("OWIRE: Received Byte!")));
        *byte = (UINT8)INREG16(&g_pOWIRE->TRX);
        bRet = TRUE;
    } else if (WAIT_TIMEOUT == dwEventRet)
    {
        DEBUGMSG(ZONE_INFO, (TEXT("OWIRE: Read Byte Timeout!!!")));
        bRet = FALSE;
    } else
    {
        DEBUGMSG(ZONE_INFO, (TEXT("OWIRE: Read Byte ERROR!!!")));
        bRet = FALSE;
    }

    DEBUGMSG(ZONE_READWRITE, (TEXT("Read byte %x\r\n"), *byte));

    OWIRE_FUNCTION_EXIT();

    return bRet;
}


static DWORD WINAPI OwireIST(LPVOID lpParam)
{
    DWORD rc = TRUE;
    USHORT wIER,wINTR;

    UNREFERENCED_PARAMETER(lpParam);

    while(!g_bTerminate)
    {
        if(WaitForSingleObject(g_hOwireInt, INFINITE) == WAIT_OBJECT_0)
        {
            wIER = INREG16(&g_pOWIRE->IER);
            wINTR = INREG16(&g_pOWIRE->INTR);
            DEBUGMSG(ZONE_INFO, (TEXT("OwireIST: ctrl: %x  Intr: %x  IER: %x\r\n"),INREG16(&g_pOWIRE->CR), wINTR, wIER));
            if ((wIER & OWIRE_ERBF_INT_EN) && (wINTR & OWIRE_RBF))
            {
                DEBUGMSG(ZONE_INFO, (TEXT("OwireIST: RBF OK!\r\n")));
                SetEvent(g_hOwireRBF);
            } else if ((wIER & OWIRE_ETSE_INT_EN) && (wINTR & OWIRE_TSRE))
            {
                DEBUGMSG(ZONE_INFO, (TEXT("OwireIST: TSRE OK!\r\n")));
                SetEvent(g_hOwireTSRE);
            } else if ((wIER & OWIRE_EPD_INT_EN) && !(wINTR & OWIRE_PDR))
            {
                DEBUGMSG(ZONE_INFO, (TEXT("OwireIST: PD OK!\r\n")));
                SetEvent(g_hOwireRPP);
            }
            InterruptDone(g_dwOwireSysIntr);
        }  else
        {
            // Abnormal interrupt signal.
            ERRORMSG(TRUE, (TEXT("Owire interrupt handler error!\r\n ")));
            rc = FALSE;
            break;
        }
    }

    return rc;

}

VOID OwireLock(void)
{
    EnterCriticalSection(&g_hOwireLock);
    DEBUGMSG(ZONE_INFO,(TEXT("TID: %d\r\n"), GetCurrentThreadId()));
}

VOID OwireUnLock(void)
{
    LeaveCriticalSection(&g_hOwireLock);
}

