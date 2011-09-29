//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  vpusdk.c
//
//  This module implements the VPU API interfaces by wrapping functions 
//  for accessing the stream interface for the VPU driver.
//
//-----------------------------------------------------------------------------


//------------------------------------------------------------------------------
// INCLUDE FILES  
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#include <csp.h>
#include "vpu_api.h"
#pragma warning(pop)
#include "vpu_io.h"
#if 0
#define VPU_FUNCTION_ENTRY() \
    printf("++%s\r\n", __FUNCTION__)
#define VPU_FUNCTION_EXIT() \
    printf("--%s\r\n", __FUNCTION__)
#else
#define VPU_FUNCTION_ENTRY()
#define VPU_FUNCTION_EXIT()
#endif

//----------------------------------------------------------------------------
// Global Variables

static HANDLE g_hVPU;

//------------------------------------------------------------------------------
//
//  Function:  vpu_Init
//
//  This function initializes the video codec module hardware.
//  
//  Parameters:  
//      No inputs
//
//  Returns:  
//      RETCODE_SUCCESS if initializing successfullly.
//      RETCODE_FAILURE for error occuring.    
// 
//------------------------------------------------------------------------------
RetCode vpu_Init()
{
    VPU_FUNCTION_ENTRY();
    if (g_hVPU != NULL) 
    {
        VPU_FUNCTION_EXIT();
        return RETCODE_CALLED_BEFORE;
    }

    g_hVPU = CreateFile(TEXT("VPU1:"),       // name of device
        GENERIC_READ|GENERIC_WRITE,         // desired access
        FILE_SHARE_READ|FILE_SHARE_WRITE,   // sharing mode
        NULL,                               // security attributes (ignored)
        OPEN_EXISTING,                      // creation disposition
        FILE_FLAG_RANDOM_ACCESS,            // flags/attributes
        NULL);                              // template file (ignored)

    // if we failed to get handle to VPU
    if (g_hVPU == INVALID_HANDLE_VALUE)
    {
        ERRORMSG(TRUE, (TEXT("CreateFile VPU failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    VPU_FUNCTION_EXIT(); 
    return RETCODE_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_Deinit
//
//  This function deinitializes the video codec module hardware.
//  
//  Parameters:  
//      No inputs
//
//  Returns:  
//      No return value   
// 
//------------------------------------------------------------------------------
RetCode  vpu_Deinit(void)
{
    VPU_FUNCTION_ENTRY();
    if (g_hVPU == NULL) 
    {
        ERRORMSG(TRUE, (TEXT("VPU isn't initialized!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_NOT_INITIALIZED;
    }

    if (!CloseHandle(g_hVPU))
    {
        ERRORMSG(TRUE, (TEXT("Close VPU handle failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    g_hVPU = NULL;
    VPU_FUNCTION_EXIT();
    return RETCODE_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_IsBusy
//
//  This function indicates if the BIT processor stops execution 
//  or starts execution  
//
//  Parameters: 
//      No inputs
//
//  Returns:
//      0 for idle; Non-zero for busy
//
//------------------------------------------------------------------------------
RetCode vpu_IsBusy()
{
    Uint32 vpubusy = 1;
    DWORD  retnum;
    VPU_FUNCTION_ENTRY();
    if (g_hVPU == NULL) 
    {
        ERRORMSG(TRUE, (TEXT("VPU isnot initialized!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_NOT_INITIALIZED;
    }

    // Issue the IOCTL to VPU busy status
    if (!DeviceIoControl(g_hVPU,   // file handle to the driver
        IOCTL_VPU_IS_BUSY,         // I/O control code
        NULL,                      // in buffer
        0,                         // in buffer size
        &vpubusy,                  // out buffer
        sizeof(Uint32),            // out buffer size
        &retnum,                   // number of bytes returned
        NULL))                     // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_VPU_IS_BUSY failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
     VPU_FUNCTION_EXIT();   

    if (vpubusy)
        return RETCODE_BUSY;
    else
        return RETCODE_IDLE;
}


//------------------------------------------------------------------------------
//
//  Function:  vpu_GetVersionInfo
//
//  This function gets the the version information of BIT Processor microcode 
//  by calling this function. 
//
//  Parameters: 
//      versionInfo
//          [out] The 16 MSB means product ID and the 16 LSB means
//                firmware version ID. 
//
//
//  Returns:
//      RETCODE_SUCCESS for success
//      RETCODE_FAILURE for failure
//      RETCODE_NOT_INITIALIZED for not initialization
//      RETCODE_FAILURE_TIMEOUT for timeout on this call
// 
//------------------------------------------------------------------------------
RetCode vpu_GetVersionInfo(Uint32 *versionInfo)
{
    DWORD  retnum;
    VPU_FUNCTION_ENTRY();    
    if (g_hVPU == NULL) 
    {
        ERRORMSG(TRUE, (TEXT("VPU isn't initialized!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_NOT_INITIALIZED;
    }

    // Issue the IOCTL to get VPU firmware version information
    if (!DeviceIoControl(g_hVPU,    // file handle to the driver
        IOCTL_VERSION_INFO,         // I/O control code
        NULL,                       // in buffer
        0,                          // in buffer size
        versionInfo,                // out buffer
        sizeof(Uint32),             // out buffer size
        &retnum,                    // number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_VERSION_INFO failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return RETCODE_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_Reset
//
//  This function has VPU to reset the instance that's specified in codecHandle
//  or with the index number specified in instIndex.
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_EncOpen/DecOpen().
//      instIndex
//          [in] the instance index to be reset
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM for invalid instance index.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong time to reset the instance .
//
//------------------------------------------------------------------------------
RetCode vpu_Reset(CodecHandle handle, int index)
{
    VpuResetInput inputparam;
    DWORD retnum;
    RetCode retcode = RETCODE_FAILURE;

    VPU_FUNCTION_ENTRY();
    if (!handle && (index > MAX_NUM_INSTANCE || index < 0))
    {
        ERRORMSG(TRUE, (TEXT("Input parameter is invalid!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_PARAM;
    }

    inputparam.codecHandle= handle;
    inputparam.instIndex = index;
    
    // Issue the IOCTL to set escape register
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_RESET_VPU,                // I/O control code
        &inputparam,                    // in buffer
        sizeof(VpuResetInput),          // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_RESET_VPU failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_DecOpen
//
//  This function opens a decoding processing instance.
//
//  Parameters: 
//      pHandle
//          [out] Pointer to the storage that contains the handle by which
//          you can refer to an decoder instance. NULL returned if no instance
//          is available.
//      pop
//          [in] Pointer to the DecOpenParam type which describes parameters
//          necessary for decoding
//
//  Returns:
//      RETCODE_SUCCESS for in acquisition of an decoder instance.
//      RETCODE_FAILURE for Failure if now more free available instance 
//      RETCODE_INVALID_PARAM for pop is a NULL pointer,or some parameters
//                      passed are invalid.
//     RETCODE_NOT_INITIALIZED for hardware being not initialized
//     RETCODE_FAILURE_TIMEOUT for timeout on function call
//
//------------------------------------------------------------------------------
RetCode vpu_DecOpen(DecHandle *pHandle, DecOpenParam *pop)
{
    DecOpenOutput outputparam = {0};
    DWORD  retnum;

    VPU_FUNCTION_ENTRY();
    if (g_hVPU == NULL) 
    {
        ERRORMSG(TRUE, (TEXT("VPU isn't initialized!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_NOT_INITIALIZED;
    }

    // Issue the IOCTL to open a decoder instance
    if (!DeviceIoControl(g_hVPU,    // file handle to the driver
        IOCTL_DEC_OPEN,             // I/O control code
        pop,                        // in buffer
        sizeof(DecOpenParam),       // in buffer size
        &outputparam,               // out buffer
        sizeof(DecOpenOutput),      // out buffer size
        &retnum,                    // number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_OPEN failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    *pHandle = outputparam.handle;
    VPU_FUNCTION_EXIT(); 
    return outputparam.retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_DecClose
//
//  This function closed the decoder instance
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_DecOpen().
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE  for invalid handle.
//      RETCODE_FAILURE_TIMEOUT for timeout on calling
//
//------------------------------------------------------------------------------
RetCode vpu_DecClose(DecHandle handle)
{
    RetCode decret = RETCODE_FAILURE;
    DWORD   retnum;
    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        return RETCODE_INVALID_HANDLE;
    }
    // Issue the IOCTL to close a decoder instance
    if (!DeviceIoControl(g_hVPU,    // file handle to the driver
        IOCTL_DEC_CLOSE,            // I/O control code
        &handle,                    // in buffer
        sizeof(DecHandle),          // in buffer size
        &decret,                    // out buffer
        sizeof(RetCode),            // out buffer size
        &retnum,                    // number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_CLOSE failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return decret;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_DecSetEscSeqInit
//
//  This function is provided to let application escape from hanging state
//  while running vpu_DecGetInitialInfo()
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_DecOpen().
//      escape
//          [in] Value to indicate if escaping from hang satus
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE  for invalid handle.
//      RETCODE_FAILURE_TIMEOUT for timeout on the calling
//
//------------------------------------------------------------------------------
RetCode vpu_DecSetEscSeqInit(DecHandle handle, int escape)
{
    RetCode retcode = RETCODE_FAILURE;
    DecSetEscSeqInitInput inputparam;
    DWORD   retnum;
    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        return RETCODE_INVALID_HANDLE;
    }

    inputparam.handle = handle;
    inputparam.escape = escape;

    // Issue the IOCTL to set escape register
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_DEC_SET_ESC_SEQ_INIT,     // I/O control code
        &inputparam,                    // in buffer
        sizeof(DecSetEscSeqInitInput),  // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_SET_ESC_SEQ_INIT failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_DecGetInitialInfo
//
//  This function gets the bitstream header information such as picture size.
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_DecOpen().
//      info
//          [out] Pointer to DecInitialInfo data structure that contains header info
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_FAILURE for failure in getting initial information
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM for invalid info pinter.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//      RETCODE_CALLED_BEFORE for the case called before.
//      RETCODE_FAILURE_TIMEOUT for timeout on the calling
//
//------------------------------------------------------------------------------
RetCode vpu_DecGetInitialInfo(DecHandle handle, DecInitialInfo *info)
{
    DecGetInitialInfoOutput outputparam;
    DWORD retnum;
    VPU_FUNCTION_ENTRY();

    if (!handle || !info)
    {
        ERRORMSG(TRUE, (TEXT("Input parameter is invalid!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_PARAM;
    }

    outputparam.intialinfo = *info;
    
    // Issue the IOCTL to get a decoder initializing information
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_DEC_GET_INITIAL_INFO,     // I/O control code
        &handle,                        // in buffer
        sizeof(DecHandle),              // in buffer size
        &outputparam,                   // out buffer
        sizeof(DecGetInitialInfoOutput),// out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_GET_INITIAL_INFO failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    *info = outputparam.intialinfo;  
    return outputparam.retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_DecRegisterFrameBuffer
//
//  This function registers the frame buffers requested by vpu_DecGetInitialInfo.
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_DecOpen().
//      bufArray
//          [in] Pointer to the first element of an array of FrameBuffer.
//      num
//          [in] Number fo elements of the array.
//      stride
//          [in] Stride value of frame buffers being registered.
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//      RETCODE_INVALID_FRAME_BUFFER Buffer for invalid bufArray pointer.
//      RETCODE_INSUFFICIENT_FRAME_BUFFERS for less num than the value requested.
//      RETCODE_INVALID_STRIDE for less stride than the picture width.
//      RETCODE_CALLED_BEFORE for the case called before.
//
//------------------------------------------------------------------------------
RetCode vpu_DecRegisterFrameBuffer(DecHandle handle,
                   FrameBuffer *bufArray, int num, int stride, DecBufInfo *pBufInfo)
{
    DecRegisterFrameBufferInput inputparam;
    RetCode retcode = RETCODE_FAILURE;
    DWORD   retnum;
    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    inputparam.handle = handle;
    inputparam.pbufarray = bufArray;
    inputparam.num = num;
    inputparam.stride = stride;
    inputparam.pbufinfo = pBufInfo;

    // Issue the IOCTL to register frame buffers and Slice save buffer
    // to a decoder
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_DEC_REGISTER_FRAME_BUF,   // I/O control code
        &inputparam,                    // in buffer
        sizeof(inputparam),             // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_REGISTER_FRAME_BUF failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();   
    return retcode;
}
//------------------------------------------------------------------------------
//
//  Function:  vpu_DecGetBitstreamBuffer
//
//  This function gets the informtion about the bitstream for decoder
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_DecOpen().
//      prdPrt
//          [out] Pointer to the storage that stores the physical address at 
//                which BIT Processor can get bitstream
//      pwrPtr
//          [out] Pointer to the storage that stores the physical address at 
//                which you can put bitstream
//      size
//          [out] Pointer to an integer that stores the size of available space.
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM for invalid bufAddr or size buffers.
//
//------------------------------------------------------------------------------
RetCode vpu_DecGetBitstreamBuffer(DecHandle handle, PhysicalAddress *prdPrt,
                PhysicalAddress *pwrPtr, Uint32 *size )
{
    DecGetBitstreamBufferOutput output = {0};
    DWORD retnum;
    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    // Issue the IOCTL to get current read and write pointers
    // and the available buffer size for input
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_DEC_GET_BITSTREAM_BUF,    // I/O control code
        &handle,                        // in buffer
        sizeof(DecHandle),              // in buffer size
        &output,                         // out buffer
        sizeof(output),                  // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_GET_BITSTREAM_BUF failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    *prdPrt = output.readptr;
    *pwrPtr = output.writeptr;
    *size = output.size;
    VPU_FUNCTION_EXIT();    
    return output.retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_DecUpdateBitstreamBuffer
//
//  This function updates the current bitstream position.
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_DecOpen().
//      size
//          [in] Size of input bitstream.
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM for invalid bitstream size.
//
//------------------------------------------------------------------------------
RetCode vpu_DecUpdateBitstreamBuffer(DecHandle handle, Uint32 size)
{
    DecUpdateBitstreamBufferInput input;
    RetCode retcode = RETCODE_FAILURE;
    DWORD retnum;
    VPU_FUNCTION_ENTRY();

    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    input.handle = handle;
    input.size = size;

    // Issue the IOCTL to update input bitstream buffer read and write pointers
    if (!DeviceIoControl(g_hVPU,               // file handle to the driver
        IOCTL_DEC_UPDATE_BITSTREAM_BUF,        // I/O control code
        &input,                                // in buffer
        sizeof(DecUpdateBitstreamBufferInput), // in buffer size
        &retcode,                              // out buffer
        sizeof(RetCode),                       // out buffer size
        &retnum,                               // number of bytes returned
        NULL))                                 // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_UPDATE_BITSTREAM_BUF failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_DecStartOneFrame
//
//  This function starts decoding one frame.
//
//  Parameters: 
//      handle
//          [in] Handle obtained form vpu_DecOpen().
//  Returns:
//      RETCODE_SUCCESS for success.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//
//------------------------------------------------------------------------------
RetCode vpu_DecStartOneFrame(DecHandle handle, DecParam *param)
{
    DecStartOneFrameInput input;
    RetCode retcode = RETCODE_FAILURE;
    DWORD   retnum;
    VPU_FUNCTION_ENTRY();
    if (!handle || !param)
    {
        ERRORMSG(TRUE, (TEXT("Invalid input parameters!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    input.handle = handle;
    input.pdecparam = param;

    // Issue the IOCTL to start a frame decoding operation
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_DEC_START_ONE_FRAME,      // I/O control code
        &input,                         // in buffer
        sizeof(DecStartOneFrameInput),  // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_START_ONE_FRAME failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return retcode;
}


//------------------------------------------------------------------------------
//
//  Function:  vpu_DecGetOutputInfo
//
//  This function gets the information of output of decoding.
//
//  Parameters: 
//      handle
//          [in] Handle obtained form vpu_DecOpen().
//      info
//          [out] Pointer to the DecOutputInfo data structure
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM Info for invalid info pointer.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//
//------------------------------------------------------------------------------
RetCode vpu_DecGetOutputInfo(DecHandle handle, DecOutputInfo *info)
{
    DecGetOutputInfoOutput output;
    DWORD retnum;

    VPU_FUNCTION_ENTRY();
    if (!handle || !info)
    {
        ERRORMSG(TRUE, (TEXT("Invalid input parameters!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    output.outputinfo = *info;

    // Issue the IOCTL to start a frame decoding operation
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_DEC_GET_OUTPUT_INFO,      // I/O control code
        &handle,                        // in buffer
        sizeof(DecHandle),              // in buffer size
        &output,                        // out buffer
        sizeof(DecGetOutputInfoOutput), // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_GET_OUTPUT_INFO failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    *info = output.outputinfo;    
    return output.retcode;
}


//------------------------------------------------------------------------------
//
//  Function:  vpu_DecBitBufferFlush
//
//  This function flushes bitstream which exist in decoder bitstream buffer 
//  without decoding of each instance.
//
//  Parameters: 
//      handle
//          [in] Handle obtained form vpu_DecOpen().
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM Info for invalid info pointer.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//
//------------------------------------------------------------------------------
RetCode vpu_DecBitBufferFlush(DecHandle handle)
{
    DWORD retnum;
    RetCode retcode = RETCODE_FAILURE;
    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }
    // Issue the IOCTL to flush the bitstream
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_DEC_BIT_BUFFER_FLUSH,     // I/O control code
        &handle,                        // in buffer
        sizeof(DecHandle),              // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_BIT_BUFFER_FLUSH failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();  
    return retcode;
}


//------------------------------------------------------------------------------
//
//  Function:  vpu_DecClrDispFlag
//
//  This function clear the display flag of each frame buffer. Application should 
//  call this function after the decoded frame is processed.
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_DecOpen().
//      index
//          [in] Index of the registered frame buffers to be cleared.
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM Info for invalid info pointer.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//
//------------------------------------------------------------------------------
RetCode vpu_DecClrDispFlag(DecHandle handle, int index)
{
    RetCode retcode = RETCODE_FAILURE;
    DecClrDispFlagInput inputparam;
    DWORD   retnum;
    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    inputparam.handle = handle;
    inputparam.index = index;

    // Issue the IOCTL to set escape register
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_DEC_CLR_DISPLAY_FLAG,      // I/O control code
        &inputparam,                    // in buffer
        sizeof(DecClrDispFlagInput),  // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_CLR_DISPLAY_FLAG failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return retcode;
}


RetCode vpu_GetDisplaytFlag(CodecHandle handle, int* piClearFlag)
{
    RetCode retcode = RETCODE_FAILURE;
    DecGetDispFlagInput inputparam;
    DWORD   retnum;
    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    inputparam.handle = handle;
    inputparam.piClearFlag = piClearFlag;

    // Issue the IOCTL to set escape register
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_VPU_GET_CLEAR_FLAG,      // I/O control code
        &inputparam,                    // in buffer
        sizeof(DecGetDispFlagInput),  // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_VPU_GET_CLEAR_FLAG failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_DecGiveCommand
//
//  This function sends the command to the decoder.
//
//  Parameters: 
//      handle
//          [in] Handle obtained form vpu_DecOpen().
//      cmd
//          [in] Command sent to decoder
//      param
//          [in/out] Parameter used for specific cmd.
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_COMMAND for invalid command
//      RETCODE_INVALID_PARAM for invalid parameter.
//      RETCODE_ROTATOR_OUTPUT_NOT_SET for no output setting.
//      RETCODE_ROTATOR_STRIDE_NOT_SET for no stride setting.
//      RETCODE_INVALID_STRIDE for invalid input stride value
//
//------------------------------------------------------------------------------
RetCode vpu_DecGiveCommand(DecHandle handle, CodecCommand cmd, void *param)
{    
    DecGiveCommandInput input;
    RetCode retcode = RETCODE_FAILURE;
    DWORD   retnum;
    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    input.handle = handle;
    input.cmd = cmd;
    input.pparam = param;

    // Issue the IOCTL to send the command to a decoder
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_DEC_GIVE_COMMAND,         // I/O control code
        &input,                         // in buffer
        sizeof(DecGiveCommandInput),    // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_DEC_GIVE_COMMAND failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_AllocPhysMem
//
//  This function allocates physically contiguous memory
//  
//  Parameters:  
//      cbSize   
//          [in] Number of bytes to allocate
//
//      VPUMemAlloc
//          [out] Pointer to a PhysicalAddress that stores 
//          the physical address of the memory allocation.
//
//  Returns:
//      RETCODE_SUCCESS for success.
//      RETCODE_FAILURE for failure
// 
//------------------------------------------------------------------------------
RetCode vpu_AllocPhysMem(Uint32 cbSize, VPUMemAlloc *pmemalloc)
{    
    DWORD   retnum;
    VPU_FUNCTION_ENTRY();
    if (g_hVPU == NULL) 
    {
        ERRORMSG(TRUE, (TEXT("VPU isn't initialized!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    // Issue the IOCTL to allocate physcial memory for VPU
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_ALLOC_PHY_MEM,            // I/O control code
        &cbSize,                        // in buffer
        sizeof(Uint32),                 // in buffer size
        pmemalloc,                       // out buffer
        sizeof(VPUMemAlloc),            // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ALLOC_PHY_MEM failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return RETCODE_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_FreePhysMem
//
//  This function releases physical memory back to the system
//  
//  Parameters:
//      VPUMemAlloc
//          [in] Pointer to a PhysicalAddress that stores 
//          the physical address of the memory allocation.
//
//  Returns:
//      RETCODE_SUCCESS for success.
//      RETCODE_FAILURE for failure
//
//------------------------------------------------------------------------------
RetCode vpu_FreePhysMem(VPUMemAlloc *pmemalloc)
{    
    DWORD   retnum;
    VPU_FUNCTION_ENTRY();
    if (g_hVPU == NULL) 
    {
        ERRORMSG(TRUE, (TEXT("VPU isn't initialized!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    // Issue the IOCTL to release physcial memory for VPU
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_FREE_PHY_MEM,             // I/O control code
        pmemalloc,                      // in buffer
        sizeof(VPUMemAlloc),            // in buffer size
        NULL,                           // out buffer
        0,                              // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_FREE_PHY_MEM failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return RETCODE_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_GetPhysAddrFromVirtAddr
//
//  This function gets physical address from inputted virtual address
//  
//  Parameters:
//      pVirtAdd
//          [in]  Pointer to a specified virtual buffer
//      size
//          [in]  Size of the virtual buffer
//      pPhysAdd
//          [out] Pointer to the pysical address of specified virtual buffer
//
//  Returns:
//      RETCODE_SUCCESS for success.
//      RETCODE_FAILURE for failure
//
//-----------------------------------------------------------------------------
RetCode vpu_GetPhysAddrFromVirtAddr(void * pVirtAdd, Uint32 size, PhysicalAddress * pPhysAdd)
{
    DWORD   retnum;
    GetPhysAddressInput input;
    VPU_FUNCTION_ENTRY();
    if (g_hVPU == NULL) 
    {
        ERRORMSG(TRUE, (TEXT("VPU isn't initialized!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_NOT_INITIALIZED;
    }

    input.pvaddress = pVirtAdd;
    input.size = size;

    // Issue the IOCTL to allocate physcial memory for VPU
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_GET_PHY_ADDR,             // I/O control code
        &input,                         // in buffer
        sizeof(GetPhysAddressInput),    // in buffer size
        pPhysAdd,                       // out buffer
        sizeof(PhysicalAddress),        // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_GET_PHY_ADDR failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    VPU_FUNCTION_EXIT();
    return RETCODE_SUCCESS;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_EncOpen
//
//  This function opens a encoding processing instance.
//
//  Parameters: 
//      pHandle
//          [out] Pointer to the storage that contains the handle by which
//          you can refer to an encoder instance. NULL returned if no instance
//          is available.
//      pop
//          [in] Pointer to the EncOpenParam type which describes parameters
//          necessary for decoding
//
//  Returns:
//      RETCODE_SUCCESS for in acquisition of an encoder instance.
//      RETCODE_FAILURE for Failure if now more free available instance 
//      RETCODE_INVALID_PARAM for pop is a NULL pointer,or some parameters
//                      passed are invalid.
//     RETCODE_NOT_INITIALIZED for hardware being not initialized
//     RETCODE_FAILURE_TIMEOUT for timeout on function call
//
//------------------------------------------------------------------------------
RetCode vpu_EncOpen(EncHandle *pHandle, EncOpenParam *pop)
{
    EncOpenOutput   outputparam = {0};
    DWORD           retnum;

    VPU_FUNCTION_ENTRY();
    if (g_hVPU == NULL) 
    {
        ERRORMSG(TRUE, (TEXT("VPU isn't initialized!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_NOT_INITIALIZED;
    }

    // Issue the IOCTL to open a decoder instance
    if (!DeviceIoControl(g_hVPU,    // file handle to the driver
        IOCTL_ENC_OPEN,             // I/O control code
        pop,                        // in buffer
        sizeof(EncOpenParam),       // in buffer size
        &outputparam,               // out buffer
        sizeof(EncOpenOutput),      // out buffer size
        &retnum,                    // number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ENC_OPEN failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    *pHandle = outputparam.handle; 
    VPU_FUNCTION_EXIT();
    return outputparam.retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_EncClose
//
//  This function closed the encoder instance
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_EncOpen().
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE  for invalid handle.
//      RETCODE_FAILURE_TIMEOUT for timeout on calling
//
//------------------------------------------------------------------------------
RetCode vpu_EncClose(EncHandle handle)
{
    RetCode encret = RETCODE_FAILURE;
    DWORD   retnum;

    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }
    // Issue the IOCTL to close a decoder instance
    if (!DeviceIoControl(g_hVPU,    // file handle to the driver
        IOCTL_ENC_CLOSE,            // I/O control code
        &handle,                    // in buffer
        sizeof(EncHandle),          // in buffer size
        &encret,                    // out buffer
        sizeof(RetCode),            // out buffer size
        &retnum,                    // number of bytes returned
        NULL))                      // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ENC_CLOSE failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    
    VPU_FUNCTION_EXIT();
    return encret;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_EncGetInitialInfo
//
//  This function gets the bitstream header information.
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_EncOpen().
//      info
//          [out] Pointer to EncInitialInfo data structure
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_FAILURE for failure in getting initial information
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM for invalid info pinter.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//      RETCODE_CALLED_BEFORE for the case called before.
//      RETCODE_FAILURE_TIMEOUT for timeout on the calling
//
//------------------------------------------------------------------------------
RetCode vpu_EncGetInitialInfo(EncHandle handle, EncInitialInfo * info)
{
    EncGetInitialInfoOutput outputparam;
    DWORD retnum;

    VPU_FUNCTION_ENTRY();
    if (!handle || !info)
    {
        ERRORMSG(TRUE, (TEXT("Input parameter is invalid!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_PARAM;
    }

    outputparam.intialinfo = *info;
    
    // Issue the IOCTL to get a decoder initializing information
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_ENC_GET_INITIAL_INFO,     // I/O control code
        &handle,                        // in buffer
        sizeof(EncHandle),              // in buffer size
        &outputparam,                   // out buffer
        sizeof(EncGetInitialInfoOutput),// out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ENC_GET_INITIAL_INFO failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    *info = outputparam.intialinfo;
    VPU_FUNCTION_EXIT();    
    return outputparam.retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_EncRegisterFrameBuffer
//
//  This function registers the frame buffers requested by vpu_EncGetInitialInfo.
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_EncOpen().
//      bufArray
//          [in] Pointer to the first element of an array of FrameBuffer.
//      num
//          [in] Number fo elements of the array.
//      stride
//          [in] Stride value of frame buffers being registered.
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//      RETCODE_INVALID_FRAME_BUFFER Buffer for invalid bufArray pointer.
//      RETCODE_INSUFFICIENT_FRAME_BUFFERS for less num than the value requested.
//      RETCODE_INVALID_STRIDE for less stride than the picture width.
//      RETCODE_CALLED_BEFORE for the case called before.
//
//------------------------------------------------------------------------------
RetCode vpu_EncRegisterFrameBuffer(EncHandle handle, FrameBuffer * bufArray, int num, int stride)
{
    EncRegisterFrameBufferInput inputparam;
    RetCode retcode = RETCODE_FAILURE;
    DWORD   retnum;
 
    VPU_FUNCTION_EXIT();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    inputparam.handle = handle;
    inputparam.pbufarray = bufArray;
    inputparam.num = num;
    inputparam.stride = stride;

    // Issue the IOCTL to register frame buffers and Slice save buffer
    // to a decoder
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_ENC_REGISTER_FRAME_BUF,   // I/O control code
        &inputparam,                    // in buffer
        sizeof(inputparam),             // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ENC_REGISTER_FRAME_BUF failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    VPU_FUNCTION_EXIT();
    return retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_EncGetBitstreamBuffer
//
//  This function gets the informtion about the bitstream for encoder
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_EncOpen().
//      prdPrt
//          [out] Pointer to the storage that stores the physical address at 
//                which BIT Processor can get bitstream
//      pwrPtr
//          [out] Pointer to the storage that stores the physical address at 
//                which you can put bitstream
//      size
//          [out] Pointer to an integer that stores the size of available space.
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM for invalid bufAddr or size buffers.
//
//------------------------------------------------------------------------------
RetCode vpu_EncGetBitstreamBuffer(EncHandle handle, PhysicalAddress * prdPrt, 
                                  PhysicalAddress * pwrPtr, Uint32 * size)
{
    EncGetBitstreamBufferOutput output;
    DWORD retnum;

    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    // Issue the IOCTL to get current read and write pointers
    // and the available buffer size for input
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_ENC_GET_BITSTREAM_BUF,    // I/O control code
        &handle,                        // in buffer
        sizeof(EncHandle),              // in buffer size
        &output,                         // out buffer
        sizeof(output),                  // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ENC_GET_BITSTREAM_BUF failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    // Remove-Prefast: Warning 6001 workaround
    PREFAST_SUPPRESS(6001,"Value is assigned in DeviceIoControl call")
    *prdPrt = output.readptr;
    *pwrPtr = output.writeptr;
    *size = output.size;
    VPU_FUNCTION_EXIT();    
    return output.retcode;
}
 
//------------------------------------------------------------------------------
//
//  Function:  vpu_EncUpdateBitstreamBuffer
//
//  This function updates the current bitstream position.
//
//  Parameters: 
//      handle
//          [in] Pointer to the handle obtained form vpu_EncOpen().
//      size
//          [in] Size of input bitstream.
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM for invalid bitstream size.
//
//------------------------------------------------------------------------------                               
RetCode vpu_EncUpdateBitstreamBuffer(EncHandle handle, Uint32 size)
{
    EncUpdateBitstreamBufferInput input;
    RetCode retcode = RETCODE_FAILURE;
    DWORD retnum;

    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    input.handle = handle;
    input.size = size;

    // Issue the IOCTL to update input bitstream buffer read and write pointers
    if (!DeviceIoControl(g_hVPU,               // file handle to the driver
        IOCTL_ENC_UPDATE_BITSTREAM_BUF,        // I/O control code
        &input,                                // in buffer
        sizeof(EncUpdateBitstreamBufferInput), // in buffer size
        &retcode,                              // out buffer
        sizeof(RetCode),                       // out buffer size
        &retnum,                               // number of bytes returned
        NULL))                                 // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ENC_UPDATE_BITSTREAM_BUF failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    VPU_FUNCTION_EXIT();
    return retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_EncStartOneFrame
//
//  This function starts encoding one frame.
//
//  Parameters: 
//      handle
//          [in] Handle obtained form vpu_EncOpen().
//  Returns:
//      RETCODE_SUCCESS for success.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//
//------------------------------------------------------------------------------
RetCode vpu_EncStartOneFrame(EncHandle handle, EncParam * param )
{
    EncStartOneFrameInput input;
    RetCode retcode = RETCODE_FAILURE;
    DWORD   retnum;
    
    VPU_FUNCTION_ENTRY();
    if (!handle || !param)
    {
        ERRORMSG(TRUE, (TEXT("Invalid input parameters!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    input.handle = handle;
    input.pencparam = param;

    // Issue the IOCTL to start a frame decoding operation
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_ENC_START_ONE_FRAME,      // I/O control code
        &input,                         // in buffer
        sizeof(EncStartOneFrameInput),  // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ENC_START_ONE_FRAME failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    VPU_FUNCTION_EXIT();
    return retcode;
}


//------------------------------------------------------------------------------
//
//  Function:  vpu_EncGetOutputInfo
//
//  This function gets the information of output of encoding.
//
//  Parameters: 
//      handle
//          [in] Handle obtained form vpu_EncOpen().
//      info
//          [out] Pointer to the EncOutputInfo data structure
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_PARAM Info for invalid info pointer.
//      RETCODE_WRONG_CALL_SEQUENCE for wrong calling sequence.
//
//------------------------------------------------------------------------------
RetCode vpu_EncGetOutputInfo(EncHandle handle, EncOutputInfo * info)
{
    EncGetOutputInfoOutput output;
    DWORD retnum;

    VPU_FUNCTION_ENTRY();
    if (!handle || !info)
    {
        ERRORMSG(TRUE, (TEXT("Invalid input parameters!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    output.outputinfo = *info;
    // Issue the IOCTL to start a frame decoding operation
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_ENC_GET_OUTPUT_INFO,      // I/O control code
        &handle,                        // in buffer
        sizeof(EncHandle),              // in buffer size
        &output,                        // out buffer
        sizeof(EncGetOutputInfoOutput), // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ENC_GET_OUTPUT_INFO failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }
    *info = output.outputinfo;
    VPU_FUNCTION_EXIT();
    return output.retcode;
}

//------------------------------------------------------------------------------
//
//  Function:  vpu_EncGiveCommand
//
//  This function sends the command to the encoder.
//
//  Parameters: 
//      handle
//          [in] Handle obtained form vpu_EncOpen().
//      cmd
//          [in] Command sent to Encoder
//      param
//          [in/out] Parameter used for specific cmd.
//  Returns:
//      RETCODE_SUCCESS for successful closing.
//      RETCODE_INVALID_HANDLE for invalid handle.
//      RETCODE_INVALID_COMMAND for invalid command
//      RETCODE_INVALID_PARAM for invalid parameter.
//      RETCODE_ROTATOR_OUTPUT_NOT_SET for no output setting.
//      RETCODE_ROTATOR_STRIDE_NOT_SET for no stride setting.
//      RETCODE_INVALID_STRIDE for invalid input stride value
//
//------------------------------------------------------------------------------
RetCode vpu_EncGiveCommand(EncHandle handle, CodecCommand cmd, void * parameter)
{
    EncGiveCommandInput input;
    RetCode retcode = RETCODE_FAILURE;
    DWORD   retnum;

    VPU_FUNCTION_ENTRY();
    if (!handle)
    {
        ERRORMSG(TRUE, (TEXT("Invalid instance!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_INVALID_HANDLE;
    }

    input.handle = handle;
    input.cmd = cmd;
    input.pparam = parameter;

    // Issue the IOCTL to send the command to a decoder
    if (!DeviceIoControl(g_hVPU,        // file handle to the driver
        IOCTL_ENC_GIVE_COMMAND,         // I/O control code
        &input,                         // in buffer
        sizeof(EncGiveCommandInput),    // in buffer size
        &retcode,                       // out buffer
        sizeof(RetCode),                // out buffer size
        &retnum,                        // number of bytes returned
        NULL))                          // ignored (=NULL)
    {
        ERRORMSG(TRUE, (TEXT("IOCTL_ENC_GIVE_COMMAND failed!\r\n")));
        VPU_FUNCTION_EXIT();
        return RETCODE_FAILURE;
    }

    VPU_FUNCTION_EXIT();
    return retcode;
}

