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
// -----------------------------------------------------------------------
// INTEL CORPORATION MAKES NO WARRANTY OF ANY KIND WITH REGARD TO THIS
// MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// INTEL CORPORATION ASSUMES NO RESPONSIBILITY FOR ANY ERRORS THAT MAY
// APPEAR IN THIS DOCUMENT. INTEL CORPORATION MAKES NO COMMITMENT TO
// UPDATE NOR TO KEEP CURRENT THE INFORMATION CONTAINED IN THIS DOCUMENT.
// -----------------------------------------------------------------------
//
// XDB Browser debug extension implementation

#define WINCEMACRO 1

#include <windows.h>
#include "xdbioctl.h"

// GLOBALS
DWORD          XSCBwrThreadID = (DWORD)INVALID_HANDLE_VALUE;
DWORD          XSCBwrProcessID = (DWORD)INVALID_HANDLE_VALUE;

// storage for the original OS DATA-Abort handler address while
// a modified execution trace handler is active 
//
static PFNVOID pfOSDataAbortHandler;

//External functions
//
extern void OEMCacheRangeFlush(LPVOID pAddr, DWORD dwLength, DWORD dwFlags);


/*-------------------------------------------------------------------------
 * Function:        GetVersionIoctl
 *-------------------------------------------------------------------------
 * Description:     process the get version command from
 *                  the XDB Browser
 *-------------------------------------------------------------------------
 * Pre/Side/Post:   prototype matches OEMIoControl
 *-------------------------------------------------------------------------
*/
static BOOL GetVersionIoctl(DWORD dwIoControlCode, 
                            LPVOID lpInBuf, DWORD nInBufSize, 
                            LPVOID lpOutBuf, DWORD nOutBufSize, 
                            LPDWORD lpBytesReturned)
{
    // output buffer format
    PXSCBwrVersion pXSCBwrVersion;

    // Return Value
    BOOL retval = FALSE;

    if ( (lpOutBuf == NULL ) ||
         (sizeof(XSCBwrVersion) > nOutBufSize) || 
         (lpBytesReturned == NULL) 
       )
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (retval);
    }

    pXSCBwrVersion = (PXSCBwrVersion)lpOutBuf;
    
    pXSCBwrVersion->Major = XSCBWR_V_MAJOR;
    pXSCBwrVersion->Minor = XSCBWR_V_MINOR;

    // Set the Bytes Returned size
    *lpBytesReturned = sizeof(XSCBwrVersion);

    // Set the return value
    retval = TRUE;

    return (retval);
}

/*-------------------------------------------------------------------------
 * Function:        ReadCoProcessorIoctl
 *-------------------------------------------------------------------------
 * Description:     process the coprocessor read command from
 *                  the XDB Browser
 *-------------------------------------------------------------------------
 * Pre/Side/Post:   prototype matches OEMIoControl
 *-------------------------------------------------------------------------
*/
static BOOL ReadCoProcessorIoctl(DWORD dwIoControlCode, 
                                 LPVOID lpInBuf, DWORD nInBufSize, 
                                 LPVOID lpOutBuf, DWORD nOutBufSize, 
                                 LPDWORD lpBytesReturned)
{
    // CoProc Read Input Buffer Ptr
    PXSCBwrRdRegIn pXSCBwrRdRegInBuf;

    // CoProc Read Output Buffer Ptr
    PXSCBwrRdRegOut pXSCBwrRdRegOutBuf;

    // Return Value
    BOOL retval = FALSE;

    //Check the size of the input & Output buffers      
    if ((lpInBuf == NULL) || (lpOutBuf == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (retval);
    }
    if ( (sizeof(XSCBwrRdRegOut) > nOutBufSize) || (lpBytesReturned == NULL) )
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (retval);
    }

    pXSCBwrRdRegInBuf = (PXSCBwrRdRegIn)lpInBuf;
    pXSCBwrRdRegOutBuf = (PXSCBwrRdRegOut)lpOutBuf;

    // execute the 'read'
    XSCBwrExecuteCoProcCode((pXSCBwrRdRegInBuf->OpCode), 
                            (&pXSCBwrRdRegOutBuf->Reg1), 
                            (&pXSCBwrRdRegOutBuf->Reg2));

    // Set the Bytes Returned size
    *lpBytesReturned = sizeof(XSCBwrRdRegOut);

    // Set the return value
    retval = TRUE;

    return (retval);
}


/*-------------------------------------------------------------------------
 * Function:        WriteCoProcessorIoctl
 *-------------------------------------------------------------------------
 * Description:     Process the coprocessor write command from
 *                  the XDB Browser
 *-------------------------------------------------------------------------
 * Pre/Side/Post:   prototype matches OEMIoControl
 *-------------------------------------------------------------------------
*/
static BOOL WriteCoProcessorIoctl(DWORD dwIoControlCode, 
                                  LPVOID lpInBuf, DWORD nInBufSize,
                                  LPVOID lpOutBuf, DWORD nOutBufSize, 
                                  LPDWORD lpBytesReturned)
{
    // CoProc Write Input Buffer Ptr
    PXSCBwrWrteRegIn pXSCBwrWrteRegInBuf;

    // Set the default return value
    BOOL retval = FALSE;

    //Check the size of the input & Output buffers (no output buffer for write)
    //
    if ((lpInBuf == NULL) || (nInBufSize < (sizeof(XSCBwrWrteRegIn)) ) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (retval);
    }

    //Now assign the input buffer
    pXSCBwrWrteRegInBuf = (PXSCBwrWrteRegIn)lpInBuf;

    // execute the 'write'
    //
    XSCBwrExecuteCoProcCode((pXSCBwrWrteRegInBuf->OpCode), 
                            (&pXSCBwrWrteRegInBuf->Reg1), 
                            (&pXSCBwrWrteRegInBuf->Reg2));

    // Set the Bytes Returned size
    *lpBytesReturned = 0;

    // Set the return value
    retval = TRUE;

    return (retval);

}

/*-------------------------------------------------------------------------
 * Function:        SetTaskIoctl
 *-------------------------------------------------------------------------
 * Description:     Set the thread context for context registers and trace 
 *-------------------------------------------------------------------------
 * Pre/Side/Post:   prototype matches OEMIoControl
 *-------------------------------------------------------------------------
*/
static BOOL SetTaskIoctl(DWORD dwIoControlCode, 
                         LPVOID lpInBuf, DWORD nInBufSize,
                         LPVOID lpOutBuf, DWORD nOutBufSize, 
                         LPDWORD lpBytesReturned)
{
    // Set Task Input Buffer Ptr
    PXSCBwrSetTaskIn pXSCBwrSetTaskInBuf;

    // Set the default return value 
    BOOL retval = FALSE;

    //Check the size of the input buffer

    if ((lpInBuf == NULL) || (sizeof(XSCBwrSetTaskIn) > nInBufSize))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (retval);
    }
    if ( (lpBytesReturned == NULL) )
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (retval);
    }

    pXSCBwrSetTaskInBuf = (PXSCBwrSetTaskIn)lpInBuf;

    if ( pXSCBwrSetTaskInBuf->ThreadID ) {
      XSCBwrThreadID = pXSCBwrSetTaskInBuf->ThreadID;
      XSCBwrProcessID = pXSCBwrSetTaskInBuf->ProcessID;
    } else {
      // thread set to 0: Browser is detaching from thread
      //
      XSCBwrThreadID = XSCBwrProcessID = (DWORD)INVALID_HANDLE_VALUE;
    }

    // Set the Bytes Returned size
    *lpBytesReturned = 0;

    // Set the return value
    retval = TRUE;

    return (retval);
}


/*-------------------------------------------------------------------------
 * Function:        ReadKMemIoctl
 *-------------------------------------------------------------------------
 * Description:     Read memory from kernel space (>=0x80000000)
 *-------------------------------------------------------------------------
 * Pre/Side/Post:   prototype matches OEMIoControl
 *-------------------------------------------------------------------------
*/
static BOOL ReadKMemIoctl(DWORD dwIoControlCode, 
                          LPVOID lpInBuf, DWORD nInBufSize,
                          LPVOID lpOutBuf, DWORD nOutBufSize, 
                          LPDWORD lpBytesReturned)
{
    // Set kmem read Input Buffer Ptr
    PXSCBwrRdKMem pXSCBwrRdKMem;

    // Set the default return value 
    BOOL retval = FALSE;

    //Check the size of the input buffer
    //
    if ( (lpInBuf == NULL) || (sizeof(XSCBwrRdKMem) > nInBufSize) )  {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (retval);
    }

    if ( (lpBytesReturned == NULL) ){
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (retval);
    }
    pXSCBwrRdKMem = (PXSCBwrRdKMem)lpInBuf;
    
    if ( (pXSCBwrRdKMem->Address & 0x80000000) == 0 ) {
      SetLastError(ERROR_INVALID_PARAMETER);  // not in kernel space
      return (retval);
    }

    // check output buffer
    //
    if ( (lpOutBuf == NULL) || (nOutBufSize < pXSCBwrRdKMem->Size ) ) {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return (retval);
    }
      
    // copy kernel memory into output buffer
    //
    __try {
      memcpy( lpOutBuf, (LPVOID)pXSCBwrRdKMem->Address, pXSCBwrRdKMem->Size );
      // Set the Bytes Returned size
      *lpBytesReturned = pXSCBwrRdKMem->Size;
      
      // Set the return value
      retval = TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
      SetLastError(ERROR_INVALID_PARAMETER);
      *lpBytesReturned = 0;
    }

    return (retval);
}


///////////////////////// EXECUTION TRACE BUFFER HANDLING /////////////////////

typedef enum e_tracemode{
        FILL_ONCE,
        WRAP_AROUND,
        TRACE_OFF
}tracemode;

typedef struct s_multi_trace{
        int TASKID;
        int CHKPT0;
        int CHKPT1;
        char Trace[256];
}multi_trace;



#define  uiAddr DWORD
#define  uiSize DWORD

typedef struct s_BufferAddr{
        uiAddr  pTraceBuffer;    //pointer to Trace buffer
        uiAddr  StartAddress;    
        uiAddr  EndAddress;
        uiSize  TraceBufferSize; //Size of Trace buffer
        tracemode TraceMode;     //Current Trace mode   
        uiAddr  TaskIDAddress;   //pointer to TaskID block
        uiSize  TaskIDSize;      //Count of Available TaskIDs
        uiSize  CurrentTaskIDs;  //Count of current used TaskIDs
}ConfigBlock;

static DWORD CurrentTaskID=0;
static int CFGBlockvalid = FALSE;
static ConfigBlock CFGBlock;


/*******************************************************************
                                        trace_clear_current
*********************************************************************/
static void trace_clear_current(void)
{ 
  //clear the Target trace buffer by reading 
  unsigned char dummy;
  int i = 0;    
  for(i=0;i<256;i++){
    dummy = (unsigned char)XSCBwrReadTraceByte();
  }
}

/*-------------------------------------------------------------------------
 * Function:        XSCBwrInitExecutionTrace
 *-------------------------------------------------------------------------
 * Description:     initialize the trace module
 *                  *pBuffer   address of the application trace buffer
 *                  Size       Size of the  buffer in Bytes (X*264 Bytes)
 *-------------------------------------------------------------------------
 * Pre/Side/Post:   prototype matches OEMIoControl
 *-------------------------------------------------------------------------
*/
void XSCBwrInitExecutionTrace(void* pBuffer,uiSize Size)
{       
    CFGBlock.TraceBufferSize = Size;
    CFGBlock.pTraceBuffer = (int) pBuffer;      
    CFGBlock.StartAddress = (int) CFGBlock.pTraceBuffer;
    CFGBlock.EndAddress   = (int) CFGBlock.pTraceBuffer;
    CFGBlock.TaskIDAddress = 0;
    CFGBlock.TaskIDSize = 0;
    CFGBlock.CurrentTaskIDs = 0;

    /* obtain address of the orginal OS DataAbort handler */
    pfOSDataAbortHandler =  NKSetDataAbortHandler(XSCBwrTraceDataAbortHandler);
    NKSetDataAbortHandler(pfOSDataAbortHandler);

    /* set multi trace mode */
    CFGBlock.TraceMode = TRACE_OFF;
    XSCBwrTraceSetFillOnce();

    //clear the current processor Trace 
    trace_clear_current();      

    RETAILMSG(1, (TEXT("XSCDBG:Trace initialized: CFG:%08x  Buffer: %08x, %x\n"),&CFGBlock,CFGBlock.pTraceBuffer, Size ));
}



/*******************************************************************
                                        fillbuffer

read the target trace buffer and store it in the application buffer

                                internal function
*********************************************************************/
static void fillbuffer(char* pCurrentTraceBuffer)
{       
  register int i = 0;

  for (i=0;i<256;i++) {
    *pCurrentTraceBuffer++ = (unsigned char)XSCBwrReadTraceByte();
  }
}
/*******************************************************************
                                        trace_add_current
*********************************************************************/
static void trace_add_current(BYTE * ptracebuffer)
{       
    //add Task info
    *ptracebuffer++ = (BYTE)(CurrentTaskID >> 24);
    *ptracebuffer++ = (BYTE)(CurrentTaskID >> 16);
    *ptracebuffer++ = (BYTE)(CurrentTaskID >>  8);
    *ptracebuffer++ = (BYTE)(CurrentTaskID >>  0);

    XSCBwrSaveTrace(ptracebuffer);

}

/*****************************************************************
       trace_admin_multi


        Administration of the multiple trace buffers
        two modes are supported: fill-once and wrap-around
        fill-once means:   if the application trace buffer is full 
                           the target stops
        wrap-around means: if the application trace buffer is full the 
                           oldest entry in the application trace buffer 
                           is overwritten with the current trace buffer.
/*****************************************************************/
static int trace_admin_multi(char** pbuffer)
{
  if(CFGBlock.EndAddress == CFGBlock.StartAddress && 
     CFGBlock.StartAddress == (int) CFGBlock.pTraceBuffer){//empty buffer
    /*return address of the current trace buffer block */ 
    *pbuffer =(char*) CFGBlock.StartAddress;
    CFGBlock.EndAddress += XDBTRACEBUFFERBLOCKSIZE;
    return FALSE; //not full            
  } else {
    if(CFGBlock.EndAddress > CFGBlock.StartAddress){
      //at least one block free for the current Trace block
      if(CFGBlock.EndAddress + XDBTRACEBUFFERBLOCKSIZE == 
         CFGBlock.StartAddress + CFGBlock.TraceBufferSize) {
        *pbuffer =(char*) CFGBlock.EndAddress;
        CFGBlock.EndAddress += XDBTRACEBUFFERBLOCKSIZE;
        if(CFGBlock.TraceMode == FILL_ONCE){
          return TRUE; // full after store this block
        }else { //WRAP AROUND
          return FALSE; 
        }
      }else{
        if(CFGBlock.EndAddress >= 
           CFGBlock.StartAddress + CFGBlock.TraceBufferSize) 
          {// WRAP AROUND occur
            if(CFGBlock.TraceMode == FILL_ONCE){
              return TRUE;/*buffer already full stop trace*/
            }
            *pbuffer =(char*) CFGBlock.StartAddress;
            CFGBlock.StartAddress += XDBTRACEBUFFERBLOCKSIZE;
            CFGBlock.EndAddress = CFGBlock.StartAddress;
            
            return FALSE;
          }else{
            *pbuffer =(char*) CFGBlock.EndAddress;
            CFGBlock.EndAddress += XDBTRACEBUFFERBLOCKSIZE;
          }
      }
    } 
    else
      {// has wrapped around // Startaddress == EndAddress
        if(CFGBlock.EndAddress ==
           (int) CFGBlock.pTraceBuffer + CFGBlock.TraceBufferSize) 
          {// WRAP AROUND occur
            *pbuffer =(char*) CFGBlock.pTraceBuffer;                                    
            CFGBlock.StartAddress = (int)CFGBlock.pTraceBuffer+
                                     XDBTRACEBUFFERBLOCKSIZE;
            CFGBlock.EndAddress = CFGBlock.StartAddress;
            if(CFGBlock.TraceMode == FILL_ONCE)
              return TRUE; // full after store this block
          }else
            {
              *pbuffer =(char*) CFGBlock.EndAddress;
              CFGBlock.StartAddress += XDBTRACEBUFFERBLOCKSIZE;
              CFGBlock.EndAddress = CFGBlock.StartAddress;
            }
      }
    return FALSE;
  }
}


/*-------------------------------------------------------------------------
 * Function:        IsUserModeCapture
 *-------------------------------------------------------------------------
 * Description:     Determine if trace block contains user mode code 
 *-------------------------------------------------------------------------
 * Return:          BOOL (TRUE = buffer contains usermode code)
 *-------------------------------------------------------------------------
*/
static BOOL IsUserModeCapture( BYTE* pTrcBuf ) 
{
  register BYTE * p;
  register DWORD Address;

  for ( p = &pTrcBuf[256]; p > pTrcBuf; p-- ) {
    switch(*p & 0xF0) {
    case 0x90:  // indirect branch
    case 0xD0: // indirect branch with chkpt
      p -= 4;
      Address = (p[0] << 24) | ( p[1] << 16 ) | (p[2] << 8) | p[3];
      if ( Address < 0x80000000) {  // user mode ?
        return TRUE;
      }
      break;
    }
  }
  return FALSE;
}

/*-------------------------------------------------------------------------
 * Function:        XSCBwrHandleTraceBufferException
 *-------------------------------------------------------------------------
 * Description:     Exception handler for the trace buffer full exception
 *-------------------------------------------------------------------------
*/
void XSCBwrHandleTraceBufferException (void)
{
  char* pBuffer;
  static BYTE NewBuffer[XDBTRACEBUFFERBLOCKSIZE];

  // temporary stoage for new buffer
  //
  XSCBwrSaveTrace(&NewBuffer[4]);

  // check if usermode code is in this capture
  //
  if ( IsUserModeCapture(&NewBuffer[4]) ) {
    trace_admin_multi(&pBuffer);
    memcpy( pBuffer, NewBuffer, XDBTRACEBUFFERBLOCKSIZE);
  }
}

/*-------------------------------------------------------------------------
 * Function:        XSCBwrExecutionTraceOn
 *-------------------------------------------------------------------------
 * Description:     enable trace buffer recording
 *-------------------------------------------------------------------------
*/
void XSCBwrExecutionTraceOn(DWORD TID)
{
    if (CFGBlock.TraceMode==TRACE_OFF)
        return;

    CurrentTaskID = TID;
    XSCBwrEnableTrace();
}

/*-------------------------------------------------------------------------
 * Function:        XSCBwrExecutionTraceOff
 *-------------------------------------------------------------------------
 * Description:     disable trace buffer recording
 *-------------------------------------------------------------------------
*/
void XSCBwrExecutionTraceOff(DWORD ID)
{
    XSCBwrDisableTrace();
}

static BOOL GetTraceConfigIoctl(DWORD dwIoControlCode, 
                                LPVOID lpInBuf, DWORD nInBufSize,
                                LPVOID lpOutBuf, DWORD nOutBufSize, 
                                LPDWORD lpBytesReturned)
{
    BOOL retval = FALSE;

    // check output buffer
    //
    if ( (lpOutBuf == NULL) || (nOutBufSize < sizeof(DWORD) ) ) {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return (retval);
    }

    *(DWORD *)lpOutBuf = (DWORD)&CFGBlock;

    // Set the return value
      retval = TRUE;
  
    *lpBytesReturned = 4;

    return (retval);
}

/*-------------------------------------------------------------------------
 * Function:        TraceControlIoctl
 *-------------------------------------------------------------------------
 * Description:     Process trace commands from the XDB Browser
 *-------------------------------------------------------------------------
 * Pre/Side/Post:   prototype matches OEMIoControl
 *-------------------------------------------------------------------------
*/
static BOOL TraceControlIoctl(DWORD dwIoControlCode, 
                              LPVOID lpInBuf, DWORD nInBufSize, 
                              LPVOID lpOutBuf, DWORD nOutBufSize, 
                              LPDWORD lpBytesReturned)
{
  // Set the default return value 
  BOOL retval = FALSE;
  
  if ( (lpInBuf == NULL) || (nInBufSize < 9) || (lpBytesReturned == NULL) )
    {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return (retval);
    }
  switch( ((char *)lpInBuf)[8] ) {
  case 0: // diable trace
    CFGBlock.TraceMode = TRACE_OFF;
    NKSetDataAbortHandler( pfOSDataAbortHandler );

    // Need to flush caches to have handler written to exception area
    OEMCacheRangeFlush(0, 0, CACHE_SYNC_ALL); 

    break;

  case 1: // enable trace
    NKSetDataAbortHandler( XSCBwrTraceDataAbortHandler );

    // Need to flush caches to have handler written to exception area
    OEMCacheRangeFlush(0, 0, CACHE_SYNC_ALL); 

    CFGBlock.TraceMode = WRAP_AROUND;

    break;
  case 2: // clear trace;
    memset( (LPVOID)CFGBlock.pTraceBuffer, 0, CFGBlock.TraceBufferSize);
    CFGBlock.StartAddress = CFGBlock.pTraceBuffer;
    CFGBlock.EndAddress   = CFGBlock.pTraceBuffer;
    break;
  }

  retval = TRUE;
  *lpBytesReturned = 0;

  return retval;
}

/*-------------------------------------------------------------------------
 * Function:        ReadTraceIoctl
 *-------------------------------------------------------------------------
 * Description:     Process trace read command from the XDB Browser
 *-------------------------------------------------------------------------
 * Pre/Side/Post:   prototype matches OEMIoControl
 *-------------------------------------------------------------------------
*/
static BOOL ReadTraceIoctl(DWORD dwIoControlCode, 
                           LPVOID lpInBuf, DWORD nInBufSize,
                           LPVOID lpOutBuf, DWORD nOutBufSize, 
                           LPDWORD lpBytesReturned)
{
    // Set the default return value 
    BOOL retval = FALSE;
    if ( (lpBytesReturned == NULL) )
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (retval);
    }
 
    // check output buffer
    //
    if ( (lpOutBuf == NULL) || (nOutBufSize < 264 ) )
    {
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return (retval);
    }
      
    // copy trace buffer into user buffer
    //
    XSCBwrSaveTrace(lpOutBuf);
    *lpBytesReturned = 264;
    retval = TRUE;

    return (retval);
}


/*-------------------------------------------------------------------------
 * Function:        XSCBwrIoControl
 *-------------------------------------------------------------------------
 * Description:     Entry point for all debug extension IOCTLS
 *                  This function is called from OEMIoControl on every
 *                  browser IOCTL
 *-------------------------------------------------------------------------
 * Pre/Side/Post:   prototype matches OEMIoControl
 *-------------------------------------------------------------------------
*/
BOOL XSCBwrIoControl(
    UINT32 code, VOID *pInpBuffer, UINT32 inpSize, VOID *pOutBuffer,
    UINT32 outSize, UINT32 *pOutSize)
{
  BOOL retval = FALSE;
  
  switch( code ) {
  case IOCTL_XSDBG_READCOPROCESSOR: 
      retval =  ReadCoProcessorIoctl( code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;

  case IOCTL_XSDBG_WRITECOPROCESSOR: 
      retval = WriteCoProcessorIoctl( code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;

  case IOCTL_XSDBG_SETTASK:
      retval = SetTaskIoctl( code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;

  
/*  These tree IOCTLs are implemented by the BSP
    case IOCTL_XSDBG_QUERYCTXREGS: 
      retval = QueryContextRegistersIoctl(code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;
    
  case IOCTL_XSDBG_READCTXREGS: 

      retval = ReadContextRegisterIoctl( code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;

  case IOCTL_XSDBG_WRITECTXREGS: 
      retval = WriteContextRegisterIoctl( code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;
*/

  case IOCTL_XSDBG_TRACECONTROL:
      retval = TraceControlIoctl(code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;

  case IOCTL_XSDBG_READTRACE: 
      retval = ReadTraceIoctl( code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;

  case IOCTL_XSDBG_READKMEM:

      retval = ReadKMemIoctl( code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;
  case IOCTL_XSDBG_GETTRACECONFIG:

      retval = GetTraceConfigIoctl( code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;

  case IOCTL_XSDBG_GETVERSION:

      retval = GetVersionIoctl( code, pInpBuffer, inpSize,
                                      pOutBuffer, outSize, pOutSize);
      break;
  default:
    SetLastError(ERROR_NOT_SUPPORTED);
    break;
  }

  return retval;
}

