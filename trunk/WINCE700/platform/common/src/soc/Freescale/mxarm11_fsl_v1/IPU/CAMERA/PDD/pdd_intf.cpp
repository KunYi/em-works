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
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004-2006,2007, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#include <pm.h>

#include "Cs.h"
#include "Csmedia.h"

#include "mxarm11.h"
#pragma warning(pop)
#include <Devload.h>
#include <NKIntr.h>
#include <ceddk.h>
#include "ipu.h"
#include "PrpClass.h"
#include "CsiClass.h"

#include "CameraPDDProps.h"
#include "dstruct.h"
#include "cameradbg.h"
#include <camera.h>
#include "CameraDriver.h"

#if (UNDER_CE > 600)
#include "CameraPinDriver.h"
#else
#include "PinDriver.h"
#endif

#include "CameraPDD.h"
#include "SensorPdd.h"
#include "wchar.h"


PVOID PDD_Init( PVOID MDDContext, PPDDFUNCTBL pPDDFuncTbl )
{
    DWORD dwRet = ERROR_SUCCESS;
    CSensorPdd *pDD = new CSensorPdd();
    if ( pDD == NULL )
    {
        return NULL;
    }
    dwRet = pDD->PDDInit( MDDContext, pPDDFuncTbl );
    if( ERROR_SUCCESS != dwRet )
    {
        return NULL;
    }
    return pDD;    

}

DWORD PDD_DeInit( LPVOID PDDContext )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD != NULL )
    {
        delete pDD;
    }
    return ERROR_SUCCESS;
}

DWORD PDD_GetAdapterInfo( LPVOID PDDContext, PADAPTERINFO pAdapterInfo )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if( NULL == pDD )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->GetAdapterInfo( pAdapterInfo );
}

DWORD PDD_HandleVidProcAmpChanges( LPVOID PDDContext, DWORD dwPropId, LONG lFlags, LONG lValue)
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->HandleVidProcAmpChanges( dwPropId, lFlags, lValue );
}

DWORD PDD_HandleCamControlChanges( LPVOID PDDContext, DWORD dwPropId, LONG lFlags, LONG lValue )
{    
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->HandleCamControlChanges( dwPropId, lFlags, lValue );
}

DWORD PDD_HandleVideoControlCapsChanges( LPVOID PDDContext, LONG lModeType ,ULONG ulCaps )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->HandleVideoControlCapsChanges( lModeType, ulCaps );
}

DWORD PDD_SetPowerState( LPVOID PDDContext, CEDEVICE_POWER_STATE PowerState )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->SetPowerState( PowerState );
}


DWORD PDD_HandleAdapterCustomProperties( LPVOID PDDContext, PUCHAR pInBuf, DWORD  InBufLen, PUCHAR pOutBuf, DWORD  OutBufLen, PDWORD pdwBytesTransferred )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->HandleAdapterCustomProperties( pInBuf, InBufLen, pOutBuf, OutBufLen, pdwBytesTransferred );
}


DWORD PDD_InitSensorMode( LPVOID PDDContext, ULONG ulModeType, LPVOID ModeContext )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->InitSensorMode( ulModeType, ModeContext );
}

DWORD PDD_DeInitSensorMode( LPVOID PDDContext, ULONG ulModeType )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->DeInitSensorMode( ulModeType );
}

DWORD PDD_SetSensorState( LPVOID PDDContext, ULONG ulModeType, CSSTATE CsState )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->SetSensorState( ulModeType, CsState );
}

DWORD PDD_TakeStillPicture( LPVOID PDDContext, LPVOID pBurstModeInfo )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->TakeStillPicture( pBurstModeInfo );
}

DWORD PDD_GetSensorModeInfo( LPVOID PDDContext, ULONG ulModeType, PSENSORMODEINFO pSensorModeInfo )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->GetSensorModeInfo( ulModeType, pSensorModeInfo );
}

DWORD PDD_SetSensorModeFormat( LPVOID PDDContext, ULONG ulModeType, PCS_DATARANGE_VIDEO pCsDataRangeVideo )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->SetSensorModeFormat( ulModeType, pCsDataRangeVideo );
}

PVOID PDD_AllocateBuffer( LPVOID PDDContext, ULONG ulModeType )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return NULL;
    }

    return pDD->AllocateBuffer( ulModeType );
}

DWORD PDD_DeAllocateBuffer( LPVOID PDDContext, ULONG ulModeType, PVOID pBuffer )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->DeAllocateBuffer( ulModeType, pBuffer );
}

DWORD PDD_RegisterClientBuffer( LPVOID PDDContext, ULONG ulModeType, PVOID pBuffer )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->RegisterClientBuffer( ulModeType, pBuffer );
}

DWORD PDD_UnRegisterClientBuffer( LPVOID PDDContext, ULONG ulModeType, PVOID pBuffer )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->UnRegisterClientBuffer( ulModeType, pBuffer );
}

DWORD PDD_FillBuffer( LPVOID PDDContext, ULONG ulModeType, PUCHAR pImage )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->FillBuffer( ulModeType, pImage );
}

DWORD PDD_HandleModeCustomProperties( LPVOID PDDContext, ULONG ulModeType, PUCHAR pInBuf, DWORD  InBufLen, PUCHAR pOutBuf, DWORD  OutBufLen, PDWORD pdwBytesTransferred )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->HandleSensorModeCustomProperties( ulModeType, pInBuf, InBufLen, pOutBuf, OutBufLen, pdwBytesTransferred );
}

#if(UNDER_CE > 600)
DWORD PDD_Open( PVOID PDDContext, PVOID MDDOpenContext )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->Open(MDDOpenContext);

}

DWORD PDD_Close( PVOID PDDContext, LPVOID MDDOpenContext )
{
    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->Close(MDDOpenContext);

}


DWORD PDD_GetMetadata( LPVOID PDDContext, DWORD dwPropId, PUCHAR pOutBuf, DWORD OutBufLen, PDWORD pdwBytesTransferred )
{

    CSensorPdd *pDD = (CSensorPdd *)PDDContext;
    if ( pDD == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    return pDD->GetMetadata(dwPropId, pOutBuf, OutBufLen, pdwBytesTransferred);
}
#endif

BOOL
__stdcall
DllMain(
   HANDLE   hDllHandle, 
   DWORD    dwReason, 
   VOID   * lpReserved
   ) 
{
    BOOL bRc = TRUE;
    
    UNREFERENCED_PARAMETER( lpReserved );
    
    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH: 
        {
           DEBUGREGISTER( reinterpret_cast<HMODULE>( hDllHandle ) );
           DEBUGMSG( ZONE_INIT, ( _T("*** DLL_PROCESS_ATTACH - Current Process: 0x%x, ID: 0x%x ***\r\n"), GetCurrentProcess(), GetCurrentProcessId() ) );

           DisableThreadLibraryCalls( reinterpret_cast<HMODULE>( hDllHandle ) );

           break;
        } 

        case DLL_PROCESS_DETACH: 
        {
            DEBUGMSG( ZONE_INIT, ( _T("*** DLL_PROCESS_DETACH - Current Process: 0x%x, ID: 0x%x ***\r\n"), GetCurrentProcess(), GetCurrentProcessId() ) );

            break;
        } 
            
        case DLL_THREAD_DETACH:
        {
            break;
        }
            
        case DLL_THREAD_ATTACH:
        {
            break;
        }
            
        default:
        {
            break;
        }
    }
    
    return bRc;
}