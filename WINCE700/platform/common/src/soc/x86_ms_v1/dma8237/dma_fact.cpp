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
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:  

    dma_fact.cpp

Abstract:  

    Contains DMA PDD Context Factory.
    
Functions:

Notes:

--*/
#include <windows.h>
#include <oaldma.h>
#include <Dma8237.hpp>

extern "C" PDMA_PDD_ADAPTER_CONTEXT CreateDMAPDDContext(LPCTSTR lpActiveRegPath, PVOID /*pReserved*/)
{
    Dma8237Adapter * pDmaAdapter = new Dma8237Adapter(lpActiveRegPath);
    if (pDmaAdapter && pDmaAdapter->Init())
        return (PDMA_PDD_ADAPTER_CONTEXT)pDmaAdapter;
    else if (pDmaAdapter)
        delete pDmaAdapter;
    return NULL;
}
extern "C" BOOL DeleteDMAPDDContext ( PDMA_PDD_ADAPTER_CONTEXT lpDmaPDDAdapterContext)
{
    Dma8237Adapter * pDmaAdapter =(Dma8237Adapter *) lpDmaPDDAdapterContext;
    if (pDmaAdapter)
        delete pDmaAdapter;
    return TRUE;
}





