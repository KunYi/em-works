/*******************************************************************************
 *
 * Copyright (c) 2007 Wolfson Microelectronics plc.  All rights reserved.
 *
 * This software as well as any related documentation may only be used or
 * copied in accordance with the terms of the Wolfson Microelectronics plc
 * agreement(s) (e.g. licence or non-disclosure agreement (NDA)).
 *
 * The information in this file is furnished for informational use only,
 * is subject to change without notice, and should not be construed as a
 * commitment by Wolfson Microelectronics plc.  Wolfson Microelectronics plc
 * assumes no responsibility or liability for any errors or inaccuracies that
 * may appear in the software or any related documentation.
 *
 * Except as permitted by the agreement(s), no part of the software or any
 * related documentation may be reproduced, stored in a retrieval system, or
 * transmitted in any form or by any means without the express written
 * consent of Wolfson Microelectronics plc.
 *                                                                         *//**
 * @file   OSAbstraction.h
 * @brief  Common definitions to hide operating system differences.
 *
 * @version $Id: OSAbstraction.h 608 2007-05-25 22:50:02Z ib $
 *
 * @Warning
 *   This software is specifically written for Wolfson devices. It may not be
 *   used with other devices.
 *
 ******************************************************************************/
#ifndef __OSABSTRACTION_H__
#define __OSABSTRACTION_H__

/*
 * Include files
 */
#if (defined _WIN32) || (defined _WIN32_WCE)   /* Windows CE - defined in project */
#   pragma warning(push)
#   pragma warning(disable: 4115)
#   pragma warning(disable: 4201)
#   pragma warning(disable: 4204)
#   pragma warning(disable: 4214)
#   include <windows.h>    /* Note: this should define WIN32. */
#   pragma warning(pop)
#   include <stdio.h>
#   if !defined( _WIN32_WCE ) && !defined( _CVI_ )
#       include <tchar.h>
#   endif
#   ifdef _CVI_
#       include <ansi_c.h>
#   endif /* _CVI_ */
#   ifndef WIN32
#       define WIN32
#   endif
#   ifdef _DEBUG
#       include <crtdbg.h>
#       define malloc( _size )   _malloc_dbg( _size, _NORMAL_BLOCK, __FILE__, __LINE__ )
#       define calloc( _num, _size)   _calloc_dbg( _num, _size, _NORMAL_BLOCK, __FILE__, __LINE__ )
#       define free( _mem )     _free_dbg( _mem, _NORMAL_BLOCK )
#   endif
#   if (defined( _MSC_VER ) && (_MSC_VER >= 1400) )
          // Turn off deprecated functions warning.
#       pragma warning( disable: 4996 )
#   endif
#   ifdef _WIN32_WCE
#       define _tfsopen( _file, _mode, _shflag )    _tfopen( _file, _mode )
#   endif
#   pragma warning( disable: 4127 ) /* Conditional expression is constant */
#endif
#include <assert.h>

/*
 * Microsoft's "safe" functions.
 */
#if !defined(_MSC_VER) || (_MSC_VER < 1400 )
#   if defined( _WIN32_WCE )
    typedef int errno_t;
#   else
#   include <errno.h>
#   endif  /* _WINCE32_WCE */
#   define _tcscpy_s( _dest, _size, _src )      _tcsncpy_s( _dest, _size, _src, _TRUNCATE )
#   define _tcstok_s( _token, _delimit, _ctx )  ( UNREFERENCED_PARAMETER( _ctx ), _tcstok( _token, _delimit ) )
#   define _tcscat_s( _dest, _size, _src )      _tcsncat_s( _dest, _size, _src, _TRUNCATE )
#   define _vsntprintf_s( _buffer, _size, _count, _format, _args )  _vsntprintf( _buffer, _size, _format, _args )
#   define _TRUNCATE    -1
#   define _SH_DENYWR   0x20
#   define _stprintf_s                          _stprintf_s_WM
#   define _sntprintf_s                         _sntprintf_s_WM
#   define _tcsncpy_s                           _tcsncpy_s_WM
#   define _tcsncat_s                           _tcsncat_s_WM
#   define _tfopen_s                            _tfopen_s_WM

/*******************************************************************************
 * Function: _stprintf_s_WM                                                *//**
 *
 * @brief   Replacement for Microsoft's _stprintf_s function.
 *
 * This is provided in cases where _stprintf_s is not available.
 *
 * @param   buffer          Buffer to print to.
 * @param   sizeOfBuffer    Number of characters buffer can hold.
 * @param   format          Format string.
 * @param   ...             Formatting arguments.
 *
 * @retval  >=0             Number of characters written.
 * @retval  <0              Error.
 ******************************************************************************/
int _stprintf_s_WM( TCHAR *buffer,
                    size_t sizeOfBuffer,
                    const TCHAR *format,
                    ...
                  );

/*******************************************************************************
 * Function: _sntprintf_s_WM                                               *//**
 *
 * @brief   Replacement for Microsoft's _sntprintf_s function.
 *
 * This is provided in cases where _sntprintf_s is not available.
 *
 * @param   buffer          Buffer to print to.
 * @param   sizeOfBuffer    Number of characters buffer can hold.
 * @param   count           Maximum number of characters to print.
 * @param   format          Format string.
 * @param   ...             Formatting arguments.
 *
 * @retval  >=0             Number of characters written.
 * @retval  <0              Error.
 ******************************************************************************/
int _sntprintf_s_WM( TCHAR *buffer,
                     size_t sizeOfBuffer,
                     size_t count,
                     const TCHAR *format,
                     ...
                   );

/*******************************************************************************
 * Function: _tcsncpy_s_WM                                                 *//**
 *
 * @brief   Replacement for Microsoft's _tcsncpy_s function.
 *
 * This is provided in cases where _tcsncpy_s is not available.
 *
 * @param   strDest             Buffer to copy to.
 * @param   numberOfElements    Maximum number of characters buffer can hold.
 * @param   strSource           Buffer to copy from.
 * @param   count               Maximum number of characters to copy.
 *
 * @retval  0               Success
 * @retval  1               Error.
 ******************************************************************************/
errno_t _tcsncpy_s_WM( TCHAR *strDest,
                       size_t numberOfElements,
                       const TCHAR *strSource,
                       size_t count
                     );

/*******************************************************************************
 * Function: _tcsncat_s_WM                                                 *//**
 *
 * @brief   Replacement for Microsoft's _tcsncat_s function.
 *
 * This is provided in cases where _tcsncat_s is not available.
 *
 * @param   strDest             Buffer to append to.
 * @param   numberOfElements    Maximum number of characters buffer can hold.
 * @param   strSource           Buffer to copy from.
 * @param   count               Maximum number of characters to copy.
 *
 * @retval  0               Success
 * @retval  1               Error.
 ******************************************************************************/
static errno_t _tcsncat_s( TCHAR *strDest,
                           size_t numberOfElements,
                           const TCHAR *strSource,
                           size_t count
                         );

/*******************************************************************************
 * Function: _tfopen_s_WM                                                  *//**
 *
 * @brief   Replacement for Microsoft's _tfopen_s function.
 *
 * This is provided in cases where _tfopen_s is not available.
 *
 * @param   ppFile          Receives file handle.
 * @param   filename        File to open.
 * @param   mode            File open mode.
 *
 * @retval  0               Success
 * @retval  1               Error.
 ******************************************************************************/
errno_t _tfopen_s_WM( FILE** ppFile,
                      const TCHAR *filename,
                      const TCHAR *mode
                    );
#endif

/*
 * Definitions
 */
#if defined(_DEBUG) && !defined(DEBUG)
#   define DEBUG    TRUE
#endif

/*
 * ASSERT is only defined on WINCE or under MFC - use assert instead elsewhere.
 */
#ifndef __AFX_H__
#   undef ASSERT
#   define ASSERT                assert
#endif

/*
 * Our internal boolean.
 */
typedef int     BOOL;
#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

/******************************************************************************
 * Mutex functions.
 */

/*
 * The type used for a mutex.
 */
typedef HANDLE                              WMMutex_t;

/*
 * CREATE_MUTEX should return WMS_SUCCESS on success, WMS_RESOURCE_FAIL on error
 */
#define CREATE_MUTEX( _pmutex, _name )      ( ( *(_pmutex) = CreateMutex( NULL, FALSE, _name ) ), *(_pmutex) ? WMS_SUCCESS : WMS_RESOURCE_FAIL )

/*
 * ACQUIRE_MUTEX should return WMS_SUCCESS on success, WMS_LOCK_TIMED_OUT on error
 */
#define ACQUIRE_MUTEX( _mutex, _timeout )   ((WAIT_OBJECT_0 == WaitForSingleObject( _mutex, _timeout )?WMS_SUCCESS:WMS_LOCK_TIMED_OUT))

/*
 * RELEASE_MUTEX  should return WMS_SUCCESS on success, WMS_FAILURE on error
 */
#define RELEASE_MUTEX( _mutex )             ( ( TRUE == ReleaseMutex( _mutex ) ) ? WMS_SUCCESS:WMS_FAILURE )

/*
 * DELETE_MUTEX  should return WMS_SUCCESS on success, WMS_FAILURE on error
 */
#define DELETE_MUTEX( _mutex )             ( ( TRUE == CloseHandle( _mutex ) ) ? WMS_SUCCESS:WMS_FAILURE )

/******************************************************************************
 * Event functions.
 */

/*
 * The type used for an event.
 */
typedef HANDLE                              WMEvent_t;

/*
 * CREATE_EVENT should return WMS_SUCCESS on success, WMS_RESOURCE_FAIL on error
 */
#define CREATE_EVENT( _pevent, _name )      ( ( *(_pevent) = CreateEvent( NULL, FALSE, FALSE, _name ) ) ? WMS_SUCCESS : WMS_RESOURCE_FAIL )

/*
 * WAIT_FOR_EVENT takes a handle and time-out interval in milliseconds.
 * Should return WMS_SUCCESS on success, WMS_EVENT_TIMED_OUT on error
 */
#define WAIT_FOR_EVENT( _event, _timeout_ms )   ( ( WAIT_OBJECT_0 == WaitForSingleObject( _event, _timeout_ms ) ? WMS_SUCCESS:WMS_EVENT_TIMED_OUT ) )

/*
 * SET_EVENT should return WMS_SUCCESS on success, WMS_FAILURE on error
 */
#define SET_EVENT( _event )                ( ( 0 == SetEvent( _event ) ) ? WMS_FAILURE:WMS_SUCCESS )

/*
 * DELETE_EVENT  should return WMS_SUCCESS on success, WMS_FAILURE on error
 */
#define DELETE_EVENT( _event )             ( ( TRUE == CloseHandle( _event ) ) ? WMS_SUCCESS:WMS_FAILURE )

/******************************************************************************
 * Atomic functions.  These perform the operation in such a way that they are
 * thread-safe, and return the new value.
 */
typedef LONG                                WMAtomic_t;
#define AtomicIncrement                     InterlockedIncrement
#define AtomicDecrement                     InterlockedDecrement

/*
 * Useful WIN32 definitions not defined elsewhere.
 */
#ifndef WIN32
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned int DWORD;
typedef void *HANDLE;
#endif
#if !defined( WIN32 ) || defined( _CVI_ )
typedef int BOOL;

/*
 * Redefinition of Windows variable-sized characters and the routines
 * to deal with them.
 */
#define TCHAR           char
#define _T( _string )   _string
#define _tprintf        printf
#define _stprintf       sprintf
#define _vsntprintf( _buf, _buflen, _fmt, _args )    vsprintf( _buf, _fmt, _args )
#define _tcslen         strlen
#define _tcscat         strcat
#define _tcschr         strchr
#define _tcscpy         strcpy
#define _tcstok         strtok
#define _tcscmp         strcmp
#define _tfopen         fopen
#define _ftprintf       fprintf
#define _fputts         fputs
#define _fgetts         fgets
#define _tmain          main
#define _tcsdup         strdup

#define wchar_t            char
#define wcslen            strlen
#define wcscat            strcat
#define wcscpy            strcpy
#define wcsdup            strdup
#define wcscmp            strcmp

#endif  /* !WIN32 || _CVI_ */

/*
 * Function prototypes
 */

#endif    /* __OSABSTRACTION_H__ */
/*------------------------------ END OF FILE ---------------------------------*/
