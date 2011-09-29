//-----------------------------------------------------------------------------
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
// Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File: ioctl.c
//
//  This file implements the OEM's IO Control (IOCTL) functions and declares
//  global variables used by the IOCTL component.
//
//-----------------------------------------------------------------------------
#include <bsp.h>
#include <usbkitl.h>

//-----------------------------------------------------------------------------
// External Functions

extern VOID CSPIRequest(PCSP_CSPI_REG pCSPI, UINT32 controlReg);
extern VOID CSPIRelease(PCSP_CSPI_REG pCSPI);

extern BOOL OALIoCtlGetResetCause(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize);

// For Watchdog Support
extern UINT32 WdogInit(UINT32 TimeoutMSec);

//
// CS&ZHL MAY-13-2011: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
extern BOOL OALFMD_Access(VOID* pInpBuffer, UINT32 inpSize);
#endif	//NAND_PDD

//-----------------------------------------------------------------------------
// External Variables
extern UINT32 g_SREV;

//-----------------------------------------------------------------------------
// Local Functions
static UINT32 OALCspiNonDMADataExchange(PCSP_CSPI_REG pCSPI, LPVOID pTxBuf, LPVOID pRxBuf, UINT32 xchCnt, DWORD dwConfigReg, BOOL bUseLoopBack);

//-----------------------------------------------------------------------------
// Global Variables
BOOL g_bOalPostInit = FALSE;

//
// CS&ZHL MAY-13-2011: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
CRITICAL_SECTION	g_oalNfcMutex;
#endif	//NAND_PDD	

//
// CS&ZHL JUN-23-2011: Built Time Stamp
//
static char			g_BuiltTimeStamp[64];
static DWORD	g_dwBuiltTimeStampLength = 0;

void DumpCspiRegs(PCSP_CSPI_REG pCSPI);
//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlPlatformType/OEM
//
//  Platform Type/OEM
//
LPCWSTR g_oalIoCtlPlatformType = IOCTL_PLATFORM_TYPE;
LPCWSTR g_oalIoCtlPlatformOEM  = IOCTL_PLATFORM_OEM;

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlProcessorVendor/Name/Core
//
//  Processor information
//
LPCWSTR g_oalIoCtlProcessorVendor = IOCTL_PROCESSOR_VENDOR;
LPCWSTR g_oalIoCtlProcessorName   = IOCTL_PROCESSOR_NAME;
LPCWSTR g_oalIoCtlProcessorCore   = IOCTL_PROCESSOR_CORE;

//------------------------------------------------------------------------------
//
//  Global: g_oalIoctlInstructionSet
//
//  Processor instruction set identifier
//
UINT32 g_oalIoCtlInstructionSet = IOCTL_PROCESSOR_INSTRUCTION_SET;
UINT32 g_oalIoCtlClockSpeed = IOCTL_PROCESSOR_CLOCK_SPEED;


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalPostInit
//
//  This function is the next OAL routine called by the kernel after OEMInit and
//  provides a context for initializing other aspects of the device prior to
//  general boot.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlHalPostInit(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize)
{  
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

	//
	// CS&ZHL JUN-24-2011: init time stamp
	//
	//sprintf(g_BuiltTimeStamp, "Emtronix Built at %s %s", __DATE__, __TIME__);
	//g_dwBuiltTimeStampLength = strlen(g_BuiltTimeStamp);
	memset(g_BuiltTimeStamp, 0x20, 64);								// set all in space
	memcpy(g_BuiltTimeStamp, 	"Emtronix Built at ", 18);
	memcpy(&g_BuiltTimeStamp[18], __DATE__, 11);			// "Jun 23 2011"
	memcpy(&g_BuiltTimeStamp[29], " ", 1);							// " "
	memcpy(&g_BuiltTimeStamp[30], __TIME__, 8);				// "12:44:02"
	g_BuiltTimeStamp[38] = '\0';
	g_dwBuiltTimeStampLength = 38;

	//
	// CS&ZHL MAY-13-2011: supporting multiple partitions of NandFlash
	//
#ifdef NAND_PDD
	RETAILMSG(1, (TEXT("OALIoCtlHalPostInit::InitializeCriticalSection(&g_oalNfcMutex)\r\n")));
	InitializeCriticalSection(&g_oalNfcMutex);
#endif	//NAND_PDD
	
	g_bOalPostInit = TRUE;
    return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlHalPresuspend
//
//  This function implements IOCTL_HAL_PRESUSPEND which provides the OAL 
//  the time needed to prepare for a suspend operation. Any preparation is 
//  completed while the system is still in threaded mode.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlHalPresuspend(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(pInpBuffer);
    UNREFERENCED_PARAMETER(inpSize);
    UNREFERENCED_PARAMETER(pOutBuffer);
    UNREFERENCED_PARAMETER(outSize);
    UNREFERENCED_PARAMETER(pOutSize);

    // Do nothing for now.
    //
    return(TRUE);
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlQueryDispSettings
//
//  This function implements IOCTL_HAL_QUERY_DISPLAYSETTINGS and is used by 
//  graphics device interface (GDI) to query the kernel for information about 
//  a preferred resolution for the system to use.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlQueryDispSettings (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);

    if (lpBytesReturned) {
        *lpBytesReturned = 0;
    }

    if (!lpOutBuf) {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else if (sizeof(DWORD)*3 > nOutBufSize) {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } else {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try {

            ((PDWORD)lpOutBuf)[0] = (DWORD) BSP_PREF_DISPLAY_WIDTH;
            ((PDWORD)lpOutBuf)[1] = (DWORD) BSP_PREF_DISPLAY_HEIGHT;
            ((PDWORD)lpOutBuf)[2] = (DWORD) BSP_PREF_DISPLAY_BPP;

            if (lpBytesReturned) {
                *lpBytesReturned = sizeof (DWORD) * 3;
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlQuerySiVersion
//
//  This function implements IOCTL_HAL_QUERY_SI_VERSION and is used to query the
//  silicon version.  The result returned by this IOCTL may be used to 
//  conditionally execute code for a specific silicon version of the SoC.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlQuerySiVersion (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);

    if (lpBytesReturned) {
        *lpBytesReturned = 0;
    }

    if (!lpOutBuf) {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else if (sizeof(DWORD) > nOutBufSize) {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } else {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try {

            // Return silicon rev from the IIM that was
            // was read during OEMInit
            *((PDWORD) lpOutBuf) = (DWORD) g_SREV;

            if (lpBytesReturned) {
                *lpBytesReturned = sizeof (DWORD);
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}


#ifdef		IMX257PDK_CPLD
//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlCpldRead16
//
//  This function implements IOCTL_HAL_CPLD_READ16 and is used to perform
//  a read operation on the debug board CPLD.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlCpldRead16 (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);

    if (lpBytesReturned) {
        *lpBytesReturned = 0;
    }

    if (!lpInBuf || nInBufSize != sizeof(UINT32) || !lpOutBuf ) {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else if (sizeof(UINT16) > nOutBufSize) {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
    } 
    else 
    {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try {

            // Perform the CPLD read
            *((PUINT16) lpOutBuf) = CPLDRead16(*((PUINT32) lpInBuf));

            if (lpBytesReturned) {
                *lpBytesReturned = sizeof(UINT16);
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlCpldWrite16
//
//  This function implements IOCTL_HAL_CPLD_WRITE16 and is used to perform
//  a write operation on the debug board CPLD.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlCpldWrite16 (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);

    if (lpBytesReturned) {
        *lpBytesReturned = 0;
    }

    if (!lpInBuf || nInBufSize != sizeof(UINT32) || 
        !lpOutBuf || nOutBufSize != sizeof(UINT16)) 
    {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else 
    {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try {

            // Perform the CPLD write
            CPLDWrite16(*((PUINT32) lpInBuf), *((PUINT16) lpOutBuf));

            if (lpBytesReturned) {
                *lpBytesReturned = sizeof(UINT16);
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}


//------------------------------------------------------------------------------
//
//  Function:  OALIoCtlSharedCspiTransfer
//
//  This function implements IOCTL_HAL_SHARED_CSPI_TRANSFER and is used to 
//  perform a protected transfer on shared CSPI port.
//
//------------------------------------------------------------------------------
BOOL OALIoCtlSharedCspiTransfer (
    UINT32 code, VOID *lpInBuf, UINT32 nInBufSize, VOID *lpOutBuf, 
    UINT32 nOutBufSize, UINT32 *lpBytesReturned) 
{
    DWORD dwErr = 0;
    PCSP_CSPI_REG pCSPI;
    UINT32 *p;

    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(code);

    if (lpBytesReturned) {
        *lpBytesReturned = 0;
    }

    if (!lpInBuf || (nInBufSize != 3*sizeof(UINT32)) || !lpOutBuf || 
        //(nOutBufSize < sizeof(UINT32)) || 
        (nOutBufSize > 64*sizeof(UINT32))) {
        dwErr = ERROR_INVALID_PARAMETER;
    } 
    else 
    {
        PREFAST_SUPPRESS(6320, "Generic exception handler");
        __try {
            
            p = (PUINT32) lpInBuf;
            pCSPI = (PCSP_CSPI_REG) OALPAtoUA(p[0]);
            if (pCSPI == NULL)
            {
                dwErr = ERROR_INVALID_PARAMETER;
            }
            else
            {                
                // Perform the CSPI transfer
                CSPIRequest(pCSPI, p[1]);

                OALCspiNonDMADataExchange(pCSPI, (LPVOID)p[3], lpOutBuf, nOutBufSize, p[1], (BOOL) p[2]);
                
                CSPIRelease(pCSPI);

                if (lpBytesReturned) {
                    *lpBytesReturned = nOutBufSize;
                }
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {
            dwErr = ERROR_INVALID_PARAMETER;
        }
    }

    if (dwErr) {
        NKSetLastError (dwErr);
    }

    return !dwErr;
}
#endif		//IMX257PDK_CPLD


//-----------------------------------------------------------------------------
//
// Function: CspiBufRd8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer, promoted to UINT32.
//
//-----------------------------------------------------------------------------
UINT32 CspiBufRd8(LPVOID pBuf)
{
    UINT8 *p;

    p = (UINT8 *) pBuf;

    return *p;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufRd16
//
// This function is used to access a buffer as an array of 16-bit (UINT16) 
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer, promoted to UINT32.
//
//-----------------------------------------------------------------------------
UINT32 CspiBufRd16(LPVOID pBuf)
{
    UINT16 *p;

    p = (UINT16 *) pBuf;

    return *p;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufRd32
//
// This function is used to access a buffer as an array of 32-bit (UINT32) 
// values and read data from the specified location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer from which data will be read.
//
// Returns:
//      Returns data pointed to by specified buffer.
//
//-----------------------------------------------------------------------------
UINT32 CspiBufRd32(LPVOID pBuf)
{
    UINT32 *p;

    p = (UINT32 *) pBuf;

    return *p;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufWrt8
//
// This function is used to access a buffer as an array of 8-bit (UINT8) 
// values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written demoted to UINT8.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CspiBufWrt8(LPVOID pBuf, UINT32 data)
{
    UINT8 *p;

    p = (UINT8 *) pBuf;

   *p = (UINT8) data;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufWrt16
//
// This function is used to access a buffer as an array of 16-bit (UINT16) 
// values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written demoted to UINT16.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CspiBufWrt16(LPVOID pBuf, UINT32 data)
{
    UINT16 *p;

    p = (UINT16 *) pBuf;

   *p = (UINT16) data;
}


//-----------------------------------------------------------------------------
//
// Function: CspiBufWrt32
//
//      This function is used to access a buffer as an array of 32-bit (UINT32) 
//      values and writes data to the specified buffer location.
//
// Parameters:
//      pBuf
//          [in] Pointer to buffer to which data will be written.
//
//      data
//          [in] Data to be written.
//
// Returns:
//      None.
//
//-----------------------------------------------------------------------------
void CspiBufWrt32(LPVOID pBuf, UINT32 data)
{
    UINT32 *p;

    p = (UINT32 *) pBuf;

   *p = data;   
}
//-----------------------------------------------------------------------------
//
// Function: CspiNonDMADataExchange
//
// Exchanges CSPI data in Application Processor(CPU) Master mode.
//
// Parameters:
//      pXchPkt
//          [in] Points to exchange packet information.
//
// Returns:
//      Returns the number of data exchanges that completed successfully.
//   
//-----------------------------------------------------------------------------
UINT32 OALCspiNonDMADataExchange(PCSP_CSPI_REG pCSPI, LPVOID pTxBuf, LPVOID pRxBuf, UINT32 xchCnt, DWORD dwConfigReg, BOOL bUseLoopBack)
{
    enum {
        LOAD_TXFIFO, 
        MAX_XCHG, 
        FETCH_RXFIFO
    } xchState;    
    UINT32 xchTxCnt = 0;
    UINT32 xchRxCnt = 0;
    volatile UINT32 tmp;
    BOOL bXchDone;
    DWORD dwOldTESTREG;
    DWORD dwTemp = 0;

    UINT32 (*pfnTxBufRd)(LPVOID) = NULL;
    void (*pfnRxBufWrt)(LPVOID, UINT32) = NULL;
    UINT8 bufIncr = 0;
    DWORD bitcount;
    
    // check all translated pointers
    if (pTxBuf == NULL)
    {
        return 0;
    }

    bitcount = EXTREG32BF(&dwConfigReg,CSPI_CONREG_BITCOUNT);
    bitcount++;

    // select access funtions based on exchange bit width
    //
    // bitcount        Tx/Rx Buffer Access Width
    // --------        -------------------------
    //   1 - 8           UINT8 (unsigned 8-bit)
    //   9 - 16          UINT16 (unsigned 16-bit)
    //  17 - 32          UINT32 (unsigned 32-bit)
    //
    if ((bitcount >= 1) && (bitcount <= 8))
    {
        // 8-bit access width
        pfnTxBufRd = CspiBufRd8;
        pfnRxBufWrt = CspiBufWrt8;
        bufIncr = sizeof(UINT8);        
        
    }
    else if ((bitcount >= 9) && (bitcount <= 16))
    {
        // 16-bit access width
        pfnTxBufRd = CspiBufRd16;
        pfnRxBufWrt = CspiBufWrt16;
        bufIncr = sizeof(UINT16);
    }
    else if (bitcount >= 17)
    {
        // 32-bit access width
        pfnTxBufRd = CspiBufRd32;
        pfnRxBufWrt = CspiBufWrt32;
        bufIncr = sizeof(UINT32);
    }
    else
    {
        // unsupported access width
        DEBUGMSG(1, (TEXT("CspiMasterDataExchange:  unsupported bitcount!\r\n")));
        return 0;
    }
    //xchCnt = xchCnt / bufIncr; // transform the transfer count from "bytes" to "number of elements (could be byte, word or dword depending on bitcount)"

    
    // disable all interrupts
    OUTREG32(&pCSPI->INTREG, 0);

    

    
    INSREG32BF(&dwConfigReg, CSPI_CONREG_EN, CSPI_CONREG_EN_DISABLE);
    INSREG32BF(&dwConfigReg, CSPI_CONREG_XCH,CSPI_CONREG_XCH_IDLE);
    INSREG32BF(&dwConfigReg, CSPI_CONREG_SMC,CSPI_CONREG_SMC_XCH);
    INSREG32BF(&dwConfigReg, CSPI_CONREG_MODE,CSPI_CONREG_MODE_MASTER);
    

    OUTREG32(&pCSPI->CONREG,dwConfigReg);


    // enable the CSPI
    dwOldTESTREG = INREG32(&pCSPI->TESTREG);
    INSREG32BF(&pCSPI->CONREG, CSPI_CONREG_EN, CSPI_CONREG_EN_ENABLE);

    if(bUseLoopBack)
    {
        INSREG32BF(&pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_CONN);
    }
    else
    {
        INSREG32BF(&pCSPI->TESTREG, CSPI_TESTREG_LBC, CSPI_TESTREG_LBC_NOCONN);
    }

    bXchDone = FALSE;
    xchState = LOAD_TXFIFO;

    //DumpCspiRegs(pCSPI);

    // until we are done with requested transfers
    while(!bXchDone)
    {
xch_loop:
        switch (xchState)
        {
            case LOAD_TXFIFO: 

                // load Tx FIFO until full, or until we run out of data
                while ((!(INREG32(&pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TF)))
                    && (xchTxCnt < xchCnt))
                {
                        // put next Tx data into CSPI FIFO
                        OUTREG32(&pCSPI->TXDATA, pfnTxBufRd(pTxBuf));

                        // increment Tx Buffer to next data point
                        pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);

                        // increment Tx exchange counter
                        xchTxCnt++;
                }
                
                // start exchange
                INSREG32(&pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_XCH), 
                    CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_EN));

                xchState = (xchTxCnt<xchCnt)? MAX_XCHG: FETCH_RXFIFO;
                //xchState = FETCH_RXFIFO;
                break;

            case MAX_XCHG:
                
                if (xchTxCnt >= xchCnt)
                {
                    //OALMSG(1, (TEXT("MAX_XCHG 1\r\n")));
                    xchState = FETCH_RXFIFO;
                    break;
                }
                // read out data that arrived during exchange                
                {
                    // wait until RR
                    dwTemp = INREG32(&pCSPI->STATREG);
                    while (!(dwTemp & CSP_BITFMASK(CSPI_STATREG_RR)))
                    {
                        //if the transfer completed and TxFifo is full then set transmit again
                        //if((dwTemp & CSP_BITFMASK(CSPI_STATREG_TC)) && (dwTemp & CSP_BITFMASK(CSPI_STATREG_TF)))
                        //{
                        //    // start exchange
                        //    INSREG32(&pCSPI->CONREG, CSP_BITFMASK(CSPI_CONREG_XCH), 
                        //        CSP_BITFVAL(CSPI_CONREG_XCH, CSPI_CONREG_XCH_EN));
                        //
                        //}
                        
                        //if the transfer completed that means CSPI hardware run faster than CPU test it
                        //we need take all RXFIFO and go back to check if all data are actually sent.
                        if(dwTemp & CSP_BITFMASK(CSPI_STATREG_TC))
                        {
                            xchState = FETCH_RXFIFO;
                            goto xch_loop;
                        }

                        dwTemp = INREG32(&pCSPI->STATREG);
                       
                    }
                    tmp = INREG32(&pCSPI->RXDATA);

                    // if receive data is not to be discarded
                    if (pRxBuf != NULL)
                    {
                        // get next Rx data from CSPI FIFO
                        pfnRxBufWrt(pRxBuf, tmp);

                        // increment Rx Buffer to next data point
                        pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
                    }

                    // increment Rx exchange counter
                    xchRxCnt++;

                    OUTREG32(&pCSPI->TXDATA, pfnTxBufRd(pTxBuf));

                    // increment Tx Buffer to next data point
                    pTxBuf = (LPVOID) ((UINT) pTxBuf + bufIncr);

                    // increment Tx exchange counter
                    xchTxCnt++;
                }
                if (INREG32(&pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC))
                {
                    xchState = FETCH_RXFIFO;
                }
                break;

            case FETCH_RXFIFO:

                // Fetch all rxdata already in RXFIFO
                while ((INREG32(&pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
                {
                    tmp = INREG32(&pCSPI->RXDATA);

                    // if receive data is not to be discarded
                    if (pRxBuf != NULL)
                    {
                        // get next Rx data from CSPI FIFO
                        pfnRxBufWrt(pRxBuf, tmp);

                        // increment Rx Buffer to next data point
                        pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
                    }

                    // increment Rx exchange counter
                    xchRxCnt++;
                }

                // Wait all data in the chain exchaged
                {
                    // wait until transaction is complete
                    while (!(INREG32(&pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_TC)))
                        ;
                    
                }

                while ((INREG32(&pCSPI->STATREG) & CSP_BITFMASK(CSPI_STATREG_RR)))
                {
                    tmp = INREG32(&pCSPI->RXDATA);

                    // if receive data is not to be discarded
                    if (pRxBuf != NULL)
                    {
                        // get next Rx data from CSPI FIFO
                        pfnRxBufWrt(pRxBuf, tmp);

                        // increment Rx Buffer to next data point
                        pRxBuf = (LPVOID) ((UINT) pRxBuf + bufIncr);
                    }

                    // increment Rx exchange counter
                    xchRxCnt++;
                }

                // acknowledge transfer complete (w1c)
                OUTREG32(&pCSPI->STATREG, CSP_BITFMASK(CSPI_STATREG_TC));
               
                if (xchRxCnt >= xchCnt)
                {
                    // set flag to indicate requested exchange done
                    bXchDone = TRUE;
                }
                else 
                {
                    // exchange stopped, and we have received all data in RXFIFO
                    // then there MUST be some data in the TxBuf, or in the TXFIFO,
                    // or both.
                    // we return the state to restart cspi XCH. 
                    xchState = LOAD_TXFIFO;
                }

                break;

            default:
                bXchDone = TRUE;
                break;
        }
    } // end of while(!bXchDone)
    

    OUTREG32(&pCSPI->TESTREG,dwOldTESTREG);    

    return xchRxCnt;
}

void DumpCspiRegs(PCSP_CSPI_REG pCSPI)
{
    OALMSG(1, (L"DumpCspiRegs\r\n"));

    OALMSG(1, (L"DumpCspiRegs CONREG = 0x%x\r\n", pCSPI->CONREG));
    OALMSG(1, (L"DumpCspiRegs DMAREG = 0x%x\r\n", pCSPI->DMAREG));
    OALMSG(1, (L"DumpCspiRegs INTREG = 0x%x\r\n", pCSPI->INTREG));
    OALMSG(1, (L"DumpCspiRegs PERIODREG = 0x%x\r\n", pCSPI->PERIODREG));
    OALMSG(1, (L"DumpCspiRegs RXDATA = 0x%x\r\n", pCSPI->RXDATA));
    OALMSG(1, (L"DumpCspiRegs STATREG = 0x%x\r\n", pCSPI->STATREG));
    OALMSG(1, (L"DumpCspiRegs TESTREG = 0x%x\r\n", pCSPI->TESTREG));
    OALMSG(1, (L"DumpCspiRegs TXDATA = 0x%x\r\n", pCSPI->TXDATA));
}

//
// CS&ZHL MAY-13-2011: supporting multiple partitions of NandFlash
//
#ifdef NAND_PDD
BOOL OALIoCtlHalNandfmdAccess(
    UINT32 code, VOID* pInpBuffer, UINT32 inpSize, VOID* pOutBuffer, 
    UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL bResult;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pOutBuffer);
	UNREFERENCED_PARAMETER(outSize);
	UNREFERENCED_PARAMETER(pOutSize);

	//OALMSG(1, (L"->OALIoCtlHalNandfmdAccess\r\n"));

	EnterCriticalSection(&g_oalNfcMutex);
	bResult = OALFMD_Access(pInpBuffer, inpSize);
	LeaveCriticalSection(&g_oalNfcMutex);

	return bResult;
}
#endif	//NAND_PDD

//
// CS&ZHL JUN-23-2011: read various info about EM9170
//
#ifdef	EM9170
BOOL OALIoCtlHalBoardInfoRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
										VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	PEM9K_CPLD_REGS	pCPLD;
	DWORD						dwValue;
	BOOL						bResult = TRUE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize != sizeof(DWORD)))
	{
		OALMSG(1, (L"OALIoCtlHalBoardInfoRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	pCPLD = (PEM9K_CPLD_REGS)OALPAtoUA(CSP_BASE_MEM_PA_CS5);
	if(pCPLD == NULL)
	{
		OALMSG(1, (L"OALIoCtlHalBoardInfoRead: CPLD pointer map failed\r\n"));
		bResult = FALSE;
		goto exit;
	}

	dwValue = (DWORD)INREG8(&pCPLD->TypeReg);		//read board info
	memcpy(pOutBuffer, &dwValue, sizeof(DWORD));
	
	if(pOutSize != NULL)
	{
		*pOutSize = sizeof(DWORD);
	}

exit:
	return bResult;
}

BOOL OALIoCtlHalTimeStampWrite(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
										VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL	bResult = TRUE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pOutBuffer);
	UNREFERENCED_PARAMETER(outSize);
	UNREFERENCED_PARAMETER(pOutSize);

	if(!pInpBuffer || (inpSize >= sizeof(g_BuiltTimeStamp)))
	{
		OALMSG(1, (L"OALIoCtlHalTimeStampWrite: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	// copy input buffer into built timestamp string
	g_dwBuiltTimeStampLength = inpSize;
	memcpy(g_BuiltTimeStamp, pInpBuffer, g_dwBuiltTimeStampLength);
	g_BuiltTimeStamp[g_dwBuiltTimeStampLength] = '\0';

exit:
	return bResult;
}

BOOL OALIoCtlHalTimeStampRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
										VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	BOOL	bResult = TRUE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize < (g_dwBuiltTimeStampLength + 1)))
	{
		OALMSG(1, (L"OALIoCtlHalTimeStampRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	// copy built timestamp string to output buffer
	memcpy(pOutBuffer, g_BuiltTimeStamp, g_dwBuiltTimeStampLength);
	
	if(pOutSize != NULL)
	{
		*pOutSize = g_dwBuiltTimeStampLength;
	}

exit:
	return bResult;
}

BOOL OALIoCtlHalVendorIDRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
										VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	PEM9K_CPLD_REGS	pCPLD;
	DWORD						dwValue;
	BOOL						bResult = TRUE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize != sizeof(DWORD)))
	{
		OALMSG(1, (L"OALIoCtlHalVendorIDRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	pCPLD = (PEM9K_CPLD_REGS)OALPAtoUA(CSP_BASE_MEM_PA_CS5);
	if(pCPLD == NULL)
	{
		OALMSG(1, (L"OALIoCtlHalVendorIDRead: CPLD pointer map failed\r\n"));
		bResult = FALSE;
		goto exit;
	}

	dwValue = 0;
	//read vendor ID
	dwValue = (dwValue << 8) | INREG8(&pCPLD->VID[3]);		
	dwValue = (dwValue << 8) | INREG8(&pCPLD->VID[2]);		
	dwValue = (dwValue << 8) | INREG8(&pCPLD->VID[1]);		
	dwValue = (dwValue << 8) | INREG8(&pCPLD->VID[0]);		
	
	memcpy(pOutBuffer, &dwValue, sizeof(DWORD));
	
	if(pOutSize != NULL)
	{
		*pOutSize = sizeof(DWORD);
	}

exit:
	return bResult;
}

BOOL OALIoCtlHalCustomerIDRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
											VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	PEM9K_CPLD_REGS	pCPLD;
	DWORD						dwValue;
	BOOL						bResult = TRUE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize != sizeof(DWORD)))
	{
		OALMSG(1, (L"OALIoCtlHalCustomerIDRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	pCPLD = (PEM9K_CPLD_REGS)OALPAtoUA(CSP_BASE_MEM_PA_CS5);
	if(pCPLD == NULL)
	{
		OALMSG(1, (L"OALIoCtlHalCustomerIDRead: CPLD pointer map failed\r\n"));
		bResult = FALSE;
		goto exit;
	}

	dwValue = 0;
	//read vendor ID
	dwValue = (dwValue << 8) | INREG8(&pCPLD->UID[3]);		
	dwValue = (dwValue << 8) | INREG8(&pCPLD->UID[2]);		
	dwValue = (dwValue << 8) | INREG8(&pCPLD->UID[1]);		
	dwValue = (dwValue << 8) | INREG8(&pCPLD->UID[0]);		
	
	memcpy(pOutBuffer, &dwValue, sizeof(DWORD));
	
	if(pOutSize != NULL)
	{
		*pOutSize = sizeof(DWORD);
	}

exit:
	return bResult;
}

BOOL OALIoCtlHalBoardStateRead(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
											VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	PEM9K_CPLD_REGS		pCPLD;
	DWORD							dwValue;
	BOOL							bResult = TRUE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize != sizeof(DWORD)))
	{
		OALMSG(1, (L"OALIoCtlHalBoardStateRead: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	pCPLD = (PEM9K_CPLD_REGS)OALPAtoUA(CSP_BASE_MEM_PA_CS5);
	if(pCPLD == NULL)
	{
		OALMSG(1, (L"OALIoCtlHalBoardStateRead: CPLD pointer map failed\r\n"));
		bResult = FALSE;
		goto exit;
	}

	//read board state
	dwValue = (DWORD)INREG8(&pCPLD->StateReg);		
	
	memcpy(pOutBuffer, &dwValue, sizeof(DWORD));
	
	if(pOutSize != NULL)
	{
		*pOutSize = sizeof(DWORD);
	}

exit:
	return bResult;
}

BOOL OALIoCtlHalWatchdogGet(UINT32 code, VOID* pInpBuffer, UINT32 inpSize, 
											VOID* pOutBuffer, UINT32 outSize, UINT32 *pOutSize) 
{
	PWATCHDOG_INFO		pWDT;
	BOOL							bResult = TRUE;

	UNREFERENCED_PARAMETER(code);
	UNREFERENCED_PARAMETER(pInpBuffer);
	UNREFERENCED_PARAMETER(inpSize);

	if(!pOutBuffer || (outSize != sizeof(WATCHDOG_INFO)))
	{
		OALMSG(1, (L"OALIoCtlHalWatchdogGet: parameter error\r\n"));
		bResult = FALSE;
		goto exit;
	}

	if((pfnOEMRefreshWatchDog != NULL) && (dwOEMWatchDogPeriod != 0))
	{
		//
		// call OEMInitWatchDogTimer(DWORD dwWatchdogPeriod) before
		//
		pfnOEMRefreshWatchDog( );		//refresh WDT first

		// transfer kernel wdt info out
		pWDT = (PWATCHDOG_INFO)pOutBuffer;
		pWDT->pfnKickWatchDog = pfnOEMRefreshWatchDog;
		pWDT->dwWatchDogPeriod = dwOEMWatchDogPeriod;
		pWDT->dwWatchDogThreadPriority = 100;						//dwNKWatchDogThreadPriority;

		//pfnOEMRefreshWatchDog = NULL;
		dwOEMWatchDogPeriod = 0;

		if(pOutSize != NULL)
		{
			*pOutSize = sizeof(WATCHDOG_INFO);
		}
	}

exit:
	return bResult;
}

#endif	//EM9170

//------------------------------------------------------------------------------
//
//  Global: g_oalIoCtlTable[]
//
//  IOCTL handler table. This table includes the IOCTL code/handler pairs
//  defined in the IOCTL configuration file. This global array is exported
//  via oal_ioctl.h and is used by the OAL IOCTL component.
//
const OAL_IOCTL_HANDLER g_oalIoCtlTable[] = {
#include "ioctl_tab.h"
};

//------------------------------------------------------------------------------
