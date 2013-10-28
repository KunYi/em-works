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
// Copyright (C) 2004-2006, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------


#include <windows.h>
#include <winddi.h>
#include <gpe.h>
#include <pm.h>

#include "csp.h"
#include "LcdcClass.h"

 /***********************************************************************
 *
 *  FUNCTION:     LcdcClass
 *
 *  DESCRIPTION:  Constructor of LCDCSurf
 *
 *  PARAMETERS:     
 *
 *  RETURNS:        
 *
**********************************************************************/
LcdcSurf::LcdcSurf( int width, 
            int height, 
            int offset, 
            void *pBits, 
            int stride, 
            EGPEFormat format, 
            Node2D *pNode) : GPESurf(width, height, pBits, stride, format)
{
    DEBUGMSG(GPE_ZONE_CREATE, (TEXT("LCDCSurf::LCDCSurf\r\n")));

    m_pNode2D                 =      pNode;
    m_fInVideoMemory          =      TRUE;
    m_nOffsetInVideoMemory    =      offset;
#ifdef OEM_ALLOC_MEM    
    m_pLcdcSurfBuffer = NULL;
#endif
}
/***********************************************************************
 *
 *  FUNCTION:     LcdcClass
 *
 *  DESCRIPTION:  Constructor of LCDCSurf
 *
 *  PARAMETERS:     
 *
 *  RETURNS:        
 *
**********************************************************************/
#ifdef OEM_ALLOC_MEM
extern const int EGPEFormatToBpp[];
LcdcSurf::LcdcSurf(int width,
                    int        height,
                    EGPEFormat format ) : GPESurf(0, 0, 0, 0,format)
 {
    unsigned long temp;
    if (width > 0 && height > 0)
    {
        temp = ( (EGPEFormatToBpp[ format ] * width + 7 )/ 8 + 3 ) & ~3L;
        m_pLcdcSurfBuffer = malloc( temp * height);
        Init(width, height, m_pLcdcSurfBuffer, temp, format);
        m_pNode2D = NULL;
    }
    else
    {
        memset(this, 0, sizeof (this));
        }   
 }
#endif
/***********************************************************************
 *
 *  FUNCTION:     LcdcClass
 *
 *  DESCRIPTION:  Destructor of LCDCSurf
 *
 *  PARAMETERS:     
 *
 *  RETURNS:        
 *
**********************************************************************/
LcdcSurf::~LcdcSurf(void)
{
    if (m_pNode2D!=NULL) 
        m_pNode2D->Free();
#ifdef OEM_ALLOC_MEM    
     if(m_pLcdcSurfBuffer != NULL){
        free(m_pLcdcSurfBuffer);
        m_pLcdcSurfBuffer = NULL;
    }
#endif
}
/*********************************************************************
 END OF FILE
*********************************************************************/
