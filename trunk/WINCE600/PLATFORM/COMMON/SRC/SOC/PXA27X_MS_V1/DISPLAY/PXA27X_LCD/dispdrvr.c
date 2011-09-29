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
/*
** Copyright 2000-2003 Intel Corporation All Rights Reserved.
**
** Portions of the source code contained or described herein and all documents
** related to such source code (Material) are owned by Intel Corporation
** or its suppliers or licensors and is licensed by Microsoft Corporation for distribution.
** Title to the Material remains with Intel Corporation or its suppliers and licensors.
** Use of the Materials is subject to the terms of the Microsoft license agreement which accompanied the Materials.
** No other license under any patent, copyright, trade secret or other intellectual
** property right is granted to or conferred upon you by disclosure or
** delivery of the Materials, either expressly, by implication, inducement,
** estoppel or otherwise
** Some portion of the Materials may be copyrighted by Microsoft Corporation.
*/

#include <windows.h>
#include <types.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include <nkintr.h>
#include <bulverde.h>
#include <ceddk.h>
#include "memdefs.h"

#include "DispDrvr.h"
#define  DEFINE_CURSOR_GLOBALS
#include "cursor.h"

#include "xllp_defs.h"
#include "xllp_serialization.h"
#include "xllp_lcd.h"

#include "xllp_bcr.h"
#include "xllp_ost.h"
#include "xllp_dmac.h"
#include "xllp_i2c.h"
extern XLLP_STATUS_T    XllpDmacInit(void);


extern XLLP_STATUS_T XllpLCDInit(P_XLLP_LCD_T pXllpLCD);
extern void XllpLCDLoadPalette(P_XLLP_LCD_T pXllpLCD);
extern void XllpLCDSuspend(P_XLLP_LCD_T pXllpLCD, int SuspendType);
extern void XllpLCDResume(P_XLLP_LCD_T pXllpLCD);

#define NUM_FRAME_BUFFERS 1

XLLP_LCD_T XllpLCD;
XLLP_STATUS_T status;

#define FRAME_BUFFER_BASE_PHYSICAL FRAME_BUFFER_0_BASE_PHYSICAL

BOOL gDrawCursorFlag = FALSE;
BOOL gInPowerHandler = FALSE;
BOOL bDoRotation     = FALSE;

CRITICAL_SECTION displayMutex;
CRITICAL_SECTION frameDescriptorMutex;

int DispDrvr_cxScreen;
int DispDrvr_cyScreen;
int DispDrvr_cdwStride;
int activeFrameBuffer=0;
unsigned int frameBufferSize = 0;

PBYTE gDibBuffer        = NULL;     // pointer to first byte of composition buffer
PBYTE gFrameBuffer      = NULL;     // pointer to first byte of screen memory
PBYTE gBlackFrameBuffer = NULL;     // pointer to first byte of screen memory
RECT  gCursorRect;
UINT  nDisplayType;

//Frame buffer. DMA will copy from here to the LCD
DWORD g_DisplayBasePhysical;
DWORD g_DisplayBaseVirtual;

//Black Buffer. DMA will copy from here during idle/suspend IF EnableScreenBlanking RegKey is enabled
DWORD g_DisplayBlackBasePhysical;
DWORD g_DisplayBlackBaseVirtual;

BOOL g_fDisableRotation = FALSE;       //Enable/disable screen rotation
BOOL g_fEnableDMASourceSwap = FALSE;   //Enable/disable screen blacking with DMA pointer swap

volatile LCDRegs              * v_pLcdRegs                  = NULL;
volatile XLLP_CLKMGR_T        * v_pClkRegs                  = NULL;
volatile XLLP_GPIO_T          * v_pGPIORegs                 = NULL;
volatile LCD_FRAME_DESCRIPTOR * frameDescriptorCh0fd1       = NULL;
volatile LCD_FRAME_DESCRIPTOR * frameDescriptorCh0fd2       = NULL;
volatile LCD_FRAME_DESCRIPTOR * frameDescriptorCh1          = NULL;
volatile LCD_FRAME_DESCRIPTOR * frameDescriptorPalette      = NULL;
volatile LCD_FRAME_DESCRIPTOR * frameDescriptorTemp         = NULL;
volatile LCD_FRAME_DESCRIPTOR * frameDescriptorCh2_YCbCr_Y  = NULL;
volatile LCD_FRAME_DESCRIPTOR * frameDescriptorCh3_YCbCr_Cb = NULL;
volatile LCD_FRAME_DESCRIPTOR * frameDescriptorCh4_YCbCr_Cr = NULL;
volatile LCD_PALETTE          * v_pPaletteBuffer            = NULL;
volatile unsigned int         * pOSCR                       = NULL;

///
volatile unsigned int * v_pOSTRegs = NULL;
volatile unsigned int * v_pCIRegs  = NULL;
volatile unsigned int * v_pI2C     = NULL;
volatile XLLP_DMAC_T  * v_pDMAC    = NULL;

HANDLE hIntEventKnown;
HANDLE hIntEvent;
///

void LCDClearStatusReg();
void LcdSetupGPIOs();
void Cleanup();
void InitLCDController();
void EnableLCDController();
void DisableLCDController();
void InitCursor();
BOOL MapVirtualAddress();
BOOL ReadRegistryData(VOID);
void ClearFrameBuffer(unsigned * fbp, BOOL color);
void ScrollBuffer(int direction);
void ClearDMACInterrupt(int channel);

extern PVOID VirtualAllocCopy(unsigned size,char *str,PVOID pVirtualAddress);
extern PVOID VirtualAllocCopyPhysical(unsigned size,char *str,PVOID pPhysicalAddress);
extern BOOL VirtualSetAttributes(LPVOID lpvAddress, DWORD cbSize, DWORD dwNewFlags, DWORD dwMask, LPDWORD lpdwOldFlags);

unsigned int    halted       = 0;
unsigned char * fbpY;
unsigned char * fbpCr;
unsigned char * fbpCb;

//

#define _OPT_ASM

#ifdef _OPT_ASM
    void dirtyRectDump_core_ASM(WORD *pwSrc, WORD *pwDst,int rowLen, DWORD srcWidthB,
            DWORD bytesPerRow, DWORD srcMarginWidth, DWORD dstMarginWidth);

    void DirtyRectDumpPortraitLoop_C(BYTE    *pDstBuf, BYTE  *pSrcBuf, DWORD    yTop, DWORD yBottom,
            DWORD srcWidthB, DWORD bytesPerRow, DWORD bytesPerPixel, DWORD srcMarginWidth, DWORD dstMarginWidth);

    void ellipse_core_ASM(WORD srcColor,DWORD margin,DWORD width,WORD* pDstBuf);
    void DispDrvrDirtyRectDump2(LPCRECT prc,DWORD color);
    void DirtyRectDumpPortraitLoop_C_rectfill(BYTE    *pDstBuf, WORD srcColor,DWORD yTop,DWORD yBottom,
            DWORD srcWidthB, DWORD bytesPerRow, DWORD bytesPerPixel, DWORD srcMarginWidth, DWORD dstMarginWidth);
    void DispDrvrDirtyRectDump_rectfill(LPCRECT prc, DWORD color);

#endif

void DispDrvrSetDibBuffer(void *data)
{
    gDibBuffer = data;
}

void DispDrvrSetPalette (const PALETTEENTRY source[],unsigned short firstEntry,unsigned short numEntries)
{
    int i;
    int end = firstEntry + numEntries;

    // Don't walk off the end of the palette buffer.
    if (firstEntry > sizeof(source) || end >= sizeof(source))
    {
        return;
    }

    EnterCriticalSection(&frameDescriptorMutex);

    // Store the palette entries into palette ram
    for(i=firstEntry;i<end;i++)
    {
        // store 5 bits red, 6 bits green, and 5 bits blue
        v_pPaletteBuffer->palette[i] = (
            (source[i].peBlue)            >>  3    |
            ((source[i].peGreen & 0xfc)    <<  3)    |
            ((source[i].peRed    & 0xf8)    <<  8)
            );
    }

    XllpLCDLoadPalette(&XllpLCD);

    LeaveCriticalSection(&frameDescriptorMutex);
}

void DispDrvrInitialize (void)
{
    // Read display driver configuration from system registry
    ReadRegistryData();

    frameBufferSize = bpp / 8 * DispDrvr_cxScreen * DispDrvr_cyScreen;

    // Map registers, the frame buffer, and frame descriptors from Kernel mode virtual address
    // into our user mode virtual address space
    if (!MapVirtualAddress())
    {
        return;
    }

    // Initialize for use with Suspend resume macros
    XllpOstDelayMilliSeconds((XLLP_OST_T *)v_pOSTRegs, 1);

    XllpLCD.GPIO = (XLLP_VUINT32_T *) v_pGPIORegs;
    XllpLCD.CLKMan = (XLLP_VUINT32_T *) v_pClkRegs;
    XllpLCD.LCDC = (XLLP_VUINT32_T *) v_pLcdRegs;
    XllpLCD.DisplayType = nDisplayType;
    XllpLCD.FrameBufferWidth = DispDrvr_cxScreen;
    XllpLCD.FrameBufferHeight = DispDrvr_cyScreen;
    XllpLCD.BPP = BPP_16;
    XllpLCD.PixelDataFormat = PDFOR_00; //with overlays enabled use PDFOR_11 for 16bpp
    XllpLCD.CurrentPage = 0;
    XllpLCD._FRAME_BUFFER_BASE_PHYSICAL = FRAME_BUFFER_BASE_PHYSICAL;
    XllpLCD._PALETTE_BUFFER_BASE_PHYSICAL = PALETTE_BUFFER_BASE_PHYSICAL;
    XllpLCD._DMA_CHANNEL_0_FRAME_DESCRIPTOR_BASE_PHYSICAL = DMA_CHANNEL_0_FRAME_DESCRIPTOR_BASE_PHYSICAL;
    XllpLCD._DMA_CHANNEL_1_FRAME_DESCRIPTOR_BASE_PHYSICAL = DMA_CHANNEL_1_FRAME_DESCRIPTOR_BASE_PHYSICAL;
    XllpLCD._DMA_CHANNEL_0_ALT_FRAME_DESCRIPTOR_BASE_PHYSICAL = DMA_CHANNEL_0_ALT_FRAME_DESCRIPTOR_BASE_PHYSICAL;
    XllpLCD._PALETTE_FRAME_DESCRIPTOR_BASE_PHYSICAL = PALETTE_FRAME_DESCRIPTOR_BASE_PHYSICAL;
    XllpLCD.frameDescriptorCh0fd1 = frameDescriptorCh0fd1;
    XllpLCD.frameDescriptorCh0fd2 = frameDescriptorCh0fd2;
    XllpLCD.frameDescriptorCh1 = frameDescriptorCh1;
    XllpLCD.frameDescriptorPalette = frameDescriptorPalette;
    XllpLCD.frameDescriptorTemp = frameDescriptorTemp;

    InitializeCriticalSection(&displayMutex);
    InitializeCriticalSection(&frameDescriptorMutex);

    // Initialize Cursor
    InitCursor();

    ClearFrameBuffer((unsigned *)gFrameBuffer + (activeFrameBuffer * frameBufferSize), TRUE);

    if( g_fEnableDMASourceSwap )
    {
        ClearFrameBuffer((unsigned *)gBlackFrameBuffer, FALSE);
    }

    // Initialize the LCD Controller and Board Control Register
    XllpLCDInit(&XllpLCD);


    //InitRegs((XLLP_OST_T *)v_pOSTRegs, (P_XLLP_I2C_T)v_pI2C);

    XllpI2cInit((P_XLLP_I2C_T)v_pI2C, (P_XLLP_GPIO_T) v_pGPIORegs, (P_XLLP_CLKMGR_T) v_pClkRegs, (XLLP_UINT32_T) 0);

    pOSCR = v_pOSTRegs + 4;

    // Use this event to signal the IST that we now know the dynamically assigned DMA channel
    // And with that information, we know which event to wait on for the interrupt.
    hIntEventKnown = CreateEvent(NULL,FALSE,FALSE,NULL);


    RETAILMSG(1,(TEXT("Display Driver Initialization Complete\r\n")));

    return;
}

void InitCursor()
{
    gDrawCursorFlag = FALSE;
    gCursorRect.left = (DispDrvr_cxScreen - CURSOR_XSIZE) >> 1;
    gCursorRect.right = gCursorRect.left + CURSOR_XSIZE;
    gCursorRect.top = (DispDrvr_cyScreen - CURSOR_YSIZE) >> 1;
    gCursorRect.bottom = gCursorRect.top + CURSOR_YSIZE;
    gxHot = gyHot = 0;
    memset ((BYTE *)gCursorMask, 0xFF, sizeof(gCursorMask));
}

BOOL MapVirtualAddress()
{
    DMA_ADAPTER_OBJECT Adapter;
    PHYSICAL_ADDRESS   PhysAddr;

    DMA_ADAPTER_OBJECT AdapterBlackScreen;
    PHYSICAL_ADDRESS   PhysAddrBlackScreen;

    Adapter.ObjectSize    = sizeof (DMA_ADAPTER_OBJECT);
    Adapter.InterfaceType = Internal;
    Adapter.BusNumber     = 0;

    g_DisplayBaseVirtual  = (DWORD)HalAllocateCommonBuffer(&Adapter, DISPLAY_BUFFER_SIZE, &PhysAddr, FALSE);
    g_DisplayBasePhysical = PhysAddr.LowPart;

    if (!g_DisplayBaseVirtual)
    {
        Cleanup();
        return FALSE;
    }

    // map shared virtual memory, and set the virtual ptr to that address rather than local process
    // address; from this point on, all processes (including DDraw apps) will be able to access the
    // display region using g_DisplayBaseVirtual
    g_DisplayBaseVirtual = (DWORD)VirtualAlloc(NULL, DISPLAY_BUFFER_SIZE, MEM_RESERVE, PAGE_NOACCESS);

    if(!VirtualCopy((LPVOID)g_DisplayBaseVirtual, (LPVOID)((unsigned long)g_DisplayBasePhysical >> 8), DISPLAY_BUFFER_SIZE, (PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)))
    {
        RETAILMSG(1,(TEXT("CRITICAL ERROR: display SDRAM region allocation failed")));
    }

    if(g_fEnableDMASourceSwap)
    {
        AdapterBlackScreen.ObjectSize    = sizeof (DMA_ADAPTER_OBJECT);
        AdapterBlackScreen.InterfaceType = Internal;
        AdapterBlackScreen.BusNumber     = 0;

        g_DisplayBlackBaseVirtual = (DWORD)HalAllocateCommonBuffer(&AdapterBlackScreen, FRAME_BUFFER_SIZE, &PhysAddrBlackScreen, FALSE);
        g_DisplayBlackBasePhysical = PhysAddrBlackScreen.LowPart;

        if (!g_DisplayBlackBaseVirtual)
        {
            Cleanup();
            return FALSE;
        }

        g_DisplayBlackBaseVirtual = (DWORD)VirtualAlloc(NULL, FRAME_BUFFER_SIZE, MEM_RESERVE, PAGE_NOACCESS);

        if(!VirtualCopy((LPVOID)g_DisplayBlackBaseVirtual, (LPVOID)((unsigned long)g_DisplayBlackBasePhysical >> 8), FRAME_BUFFER_SIZE, (PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)))
        {
            RETAILMSG(1,(TEXT("CRITICAL ERROR: blank display SDRAM region allocation failed")));
        }
    }


    v_pDMAC = (P_XLLP_DMAC_T)VirtualAllocCopyPhysical(sizeof(XLLP_DMAC_T),"DispDrvrInitialize : v_pDMAC",(PVOID)(BULVERDE_BASE_REG_PA_DMAC));
    if (!v_pDMAC)
    {
        Cleanup();
        return FALSE;
    }

    v_pI2C = (volatile unsigned int *)VirtualAllocCopyPhysical(sizeof(BULVERDE_IICBUS_REG),"DispDrvrInitialize : v_pI2CRegs",(PVOID)(BULVERDE_BASE_REG_PA_I2C));
    if (!v_pI2C)
    {
        Cleanup();
        return FALSE;
    }

    v_pOSTRegs = (volatile unsigned int *)VirtualAllocCopyPhysical(sizeof(XLLP_OST_T),"DispDrvrInitialize : v_pOSTRegs",(PVOID)(BULVERDE_BASE_REG_PA_OST));
    if (!v_pOSTRegs)
    {
        Cleanup();
        return FALSE;
    }

    v_pLcdRegs = (volatile LCDRegs *)VirtualAllocCopyPhysical(sizeof(LCDRegs),"DispDrvrInitialize : v_pLcdRegs",(PVOID)(BULVERDE_BASE_REG_PA_LCD));
    if (!v_pLcdRegs)
    {
        Cleanup();
        return FALSE;
    }

    v_pClkRegs = (volatile XLLP_CLKMGR_T *)VirtualAllocCopyPhysical(sizeof(XLLP_CLKMGR_T),"DispDrvrInitialize : v_pClkRegs",(PVOID)(BULVERDE_BASE_REG_PA_CLKMGR));
    if (!v_pClkRegs)
    {
        Cleanup();
        return FALSE;
    }

    v_pGPIORegs = (volatile XLLP_GPIO_T *)VirtualAllocCopyPhysical(sizeof(XLLP_GPIO_T),"DispDrvrInitialize : v_pGPIORegs",(PVOID)(BULVERDE_BASE_REG_PA_GPIO));
    if (!v_pGPIORegs)
    {
        Cleanup();
        return FALSE;
    }

    frameDescriptorCh0fd1  = (volatile LCD_FRAME_DESCRIPTOR *)(DMA_CHANNEL_0_FRAME_DESCRIPTOR_BASE_VIRTUAL);
    frameDescriptorCh0fd2  = (volatile LCD_FRAME_DESCRIPTOR *)(DMA_CHANNEL_0_ALT_FRAME_DESCRIPTOR_BASE_VIRTUAL);
    frameDescriptorCh1     = (volatile LCD_FRAME_DESCRIPTOR *)(DMA_CHANNEL_1_FRAME_DESCRIPTOR_BASE_VIRTUAL);
    frameDescriptorPalette = (volatile LCD_FRAME_DESCRIPTOR *)(PALETTE_FRAME_DESCRIPTOR_BASE_VIRTUAL);
    v_pPaletteBuffer       = (volatile LCD_PALETTE *)(PALETTE_BUFFER_BASE_VIRTUAL);

    // Enter into Kernel mode to enable us to modify the section descriptor
    // so that we may set the bufferable bit.  This enables write coalescing
    // for frame buffer writes when using the section mapped address.
    //
    // GAPI uses the section mapped address always.


    // Now configure the frame buffer's section descriptor.
    // The function GetDescriptorAddress shows how to obtain the correct descriptor address.
    // This descriptor is one of two descriptors that map the the frame buffer.
    // The first descriptor found maps the cached virtual address, while the second
    // descriptor found maps the uncached virtual address.  We want to modify the
    // second descriptor, that which maps the uncached virtual address since the uncached virtual
    // address is the address we've chosen to use throughout the codebase.
    //
    // NOTE:
    // The section descriptor covers a 1MB section.  If the frame buffer ever exceeds 1MB
    // in size, you'll need to modify additional section descriptors.
    //

    // DDraw requires that the frame buffer pointer be in the shared memory space so
    // is can be shared between processes.
    {
        PVOID  pPhysAddr;
        size_t offset;
        size_t size;

        pPhysAddr  = (PVOID)(FRAME_BUFFER_0_BASE_PHYSICAL);
        size       = frameBufferSize * NUM_FRAME_BUFFERS;
        offset     = (unsigned)pPhysAddr & (0x1000 - 1);
        size      += (offset ? 0x1000 : 0);
        pPhysAddr  = (PVOID)((unsigned)pPhysAddr - offset);

        if (size >= 1024*1024*2)
        {
            gFrameBuffer = (PBYTE)VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
        }
        else
        {
            gFrameBuffer = (PBYTE)VirtualAlloc(NULL, 1024*1024*2, MEM_RESERVE, PAGE_NOACCESS);
        }

        if (!VirtualCopy(gFrameBuffer, (LPVOID)((unsigned long)pPhysAddr >> 8), size, (PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)))
        {
            gFrameBuffer = NULL;
        }
        else
        {
            gFrameBuffer += offset;
        }
    }

    if (!gFrameBuffer)
    {
        Cleanup();
        return FALSE;
    }

    if (bDoRotation)
    {
        // if rotating the display, the actual frame buffer should be configured as bufferable for max write performance into the frame buffer.
        VirtualSetAttributes(gFrameBuffer, frameBufferSize*NUM_FRAME_BUFFERS, 4, 4, NULL);
    }
    else
    {
        // if not rotating the dispay, we can draw directly into the frame buffer, and use write-through cache mode to improve frame buffer throughput
        VirtualSetAttributes(gFrameBuffer, frameBufferSize*NUM_FRAME_BUFFERS, 8, 8, NULL);
    }

    if (g_fEnableDMASourceSwap) {
        PVOID  pPhysAddr;
        size_t offset;
        size_t size;

        pPhysAddr  = (PVOID)(g_DisplayBlackBasePhysical);
        size       = FRAME_BUFFER_SIZE;
        offset     = (unsigned)pPhysAddr & (0x1000 - 1);
        size      += (offset ? 0x1000 : 0);
        pPhysAddr  = (PVOID)((unsigned)pPhysAddr - offset);

        gBlackFrameBuffer = (PBYTE)VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);

        if (!VirtualCopy(gBlackFrameBuffer, (LPVOID)((unsigned long)pPhysAddr >> 8), size, (PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL)))
        {
            gBlackFrameBuffer = NULL;
        }
        else
        {
            gBlackFrameBuffer += offset;
        }

        if(!gBlackFrameBuffer) {
            Cleanup();
            return FALSE;
        }
    }

    return TRUE;
}


// Free all the global memory resources
void Cleanup(void)
{
    DMA_ADAPTER_OBJECT Adapter;
    PHYSICAL_ADDRESS   PhysAddr;

    DMA_ADAPTER_OBJECT AdapterBlackScreen;
    PHYSICAL_ADDRESS   PhysAddrBlackScreen;

    if (v_pLcdRegs)
    {
        VirtualFree((PVOID)v_pLcdRegs,0,MEM_RELEASE);
        v_pLcdRegs = NULL;
    }

    if (v_pClkRegs)
    {
        VirtualFree((PVOID)v_pClkRegs,0,MEM_RELEASE);
        v_pLcdRegs = NULL;
    }

    if (v_pGPIORegs)
    {
        VirtualFree((PVOID)v_pGPIORegs,0,MEM_RELEASE);
        v_pGPIORegs = NULL;
    }

    if (gFrameBuffer)
    {
        VirtualFree((PVOID)gFrameBuffer,0,MEM_RELEASE);
        gFrameBuffer = NULL;
    }

    if (gBlackFrameBuffer)
    {
        VirtualFree((PVOID)gBlackFrameBuffer,0,MEM_RELEASE);
        gBlackFrameBuffer = NULL;
    }

    if (frameDescriptorCh2_YCbCr_Y)
    {
        VirtualFree((PVOID)frameDescriptorCh2_YCbCr_Y,0,MEM_RELEASE);
        frameDescriptorCh2_YCbCr_Y = NULL;
    }

    if (frameDescriptorCh3_YCbCr_Cb)
    {
        VirtualFree((PVOID)frameDescriptorCh3_YCbCr_Cb,0,MEM_RELEASE);
        frameDescriptorCh3_YCbCr_Cb = NULL;
    }

    if (frameDescriptorCh4_YCbCr_Cr)
    {
        VirtualFree((PVOID)frameDescriptorCh4_YCbCr_Cr,0,MEM_RELEASE);
        frameDescriptorCh4_YCbCr_Cr = NULL;
    }

    Adapter.ObjectSize    = sizeof (DMA_ADAPTER_OBJECT);
    Adapter.InterfaceType = Internal;
    Adapter.BusNumber     = 0;

    PhysAddr.HighPart = 0;
    PhysAddr.LowPart  = g_DisplayBasePhysical;

    HalFreeCommonBuffer(&Adapter, DISPLAY_BUFFER_SIZE, PhysAddr, (void *)g_DisplayBaseVirtual, FALSE);

    if( g_fEnableDMASourceSwap && g_DisplayBlackBaseVirtual)
    {
        AdapterBlackScreen.ObjectSize    = sizeof (DMA_ADAPTER_OBJECT);
        AdapterBlackScreen.InterfaceType = Internal;
        AdapterBlackScreen.BusNumber     = 0;

        PhysAddrBlackScreen.HighPart = 0;
        PhysAddrBlackScreen.LowPart  = g_DisplayBlackBasePhysical;

        HalFreeCommonBuffer(&AdapterBlackScreen, FRAME_BUFFER_SIZE, PhysAddrBlackScreen, (void *)g_DisplayBlackBaseVirtual, FALSE);
    }
}

void DispDrvrPowerHandler(BOOL    bOff)
{

    if(bOff)
    {

        if( g_fEnableDMASourceSwap )
        {
            // Before turning off, lets switch out the DMA's source address
            // Old source address: FRAME_BUFFER_BASE_PHYSICAL (in virtual land gFrameBuffer)
            // New source address: g_DisplayBlackBasePhysical (in virtual land g_DisplayBlackBaseVirtual)

            // So when we go into user idle, GWES keeps writing to the original buffer,
            // but DMA updates the LCD with the black buffer

            // Swap the DMA pointer
            XllpLCD._FRAME_BUFFER_BASE_PHYSICAL = g_DisplayBlackBasePhysical;
            XllpLCDInit(&XllpLCD);  //Let XllpLCDInit update DMA for us
            XllpOstDelayMilliSeconds((XLLP_OST_T *)v_pOSTRegs, 1);
        }

        XllpLCDSuspend(&XllpLCD, Suspend_Graceful);
    }
    else
    {
        if( g_fEnableDMASourceSwap )
        {
            // Swap back the DMA pointer
            XllpLCD._FRAME_BUFFER_BASE_PHYSICAL = FRAME_BUFFER_BASE_PHYSICAL;
            XllpLCDInit(&XllpLCD);
            XllpOstDelayMilliSeconds((XLLP_OST_T *)v_pOSTRegs, 1);
        }

        XllpLCDResume(&XllpLCD);
    }
}


void ClearFrameBuffer(unsigned * fbp, BOOL color)
{
    DWORD i;

    for(i = 0; i < (DispDrvr_cxScreen * DispDrvr_cyScreen * (bpp / 8) / 4); i++)
    {
        if (color)
        {
            *fbp++ = 0xFFFFFFFF;    // Ones turn it white
        }
        else
        {
            *fbp++ = 0x00000000;    // Zeros turn it black
        }
    }
}

//**********************************************************************
//
//DispDrvrContrastControl:
//
//    Modify the contrast according to the Cmd parameter.
// Not supported
//

BOOL DispDrvrContrastControl(int Cmd,DWORD *pValue)
{
    // currently does not support changing contrast in software.
    return TRUE;
}

void DirtyRectDumpPortraitLoop_C(BYTE    *pDstBuf, BYTE  *pSrcBuf, DWORD    yTop, DWORD yBottom,
                                 DWORD srcWidthB, DWORD    bytesPerRow, DWORD bytesPerPixel,
                                 DWORD srcMarginWidth, DWORD dstMarginWidth)
{
    DWORD row;
    DWORD i;
    DWORD j;

    if ( bytesPerPixel != 2 )
    {
        //not 16-bit
        for (i = 0; i < srcWidthB / bytesPerPixel; i++)
        {
            for (row = yTop; row < yBottom; row++)
            {
                for (j = 0; j < bytesPerPixel; j++)
                {
                    *pDstBuf++ = *(pSrcBuf + j);
                }

                pSrcBuf -= bytesPerRow;
            }

            pDstBuf += dstMarginWidth;
            pSrcBuf += srcMarginWidth + 2;
        }
    }
    else
    {
        WORD * pwDst;
        WORD * pwSrc;
        int    rowLen;

        //16-bit
        srcWidthB >>= 1;

        pwDst = (WORD *)pDstBuf;
        pwSrc = (WORD *)pSrcBuf;

        //first row for pwSrc, then column for pwDst
        rowLen = yBottom - yTop;

#ifndef _OPT_ASM
        bytesPerRow    >>= 1;
        dstMarginWidth >>= 1;
        srcMarginWidth >>= 1;

        for (i = 0; i < srcWidthB; i++)
        {
            for (row = 0; row < (rowLen >> 2); row++)
            {
                *pwDst++  = *pwSrc;
                pwSrc    -= bytesPerRow;

                *pwDst++  = *pwSrc;
                pwSrc    -= bytesPerRow;

                *pwDst++  = *pwSrc;
                pwSrc    -= bytesPerRow;

                *pwDst++  = *pwSrc;
                pwSrc    -= bytesPerRow;
            }

            for (row = 0; row < (rowLen & 0x3); row++)
            {
                *pwDst++  = *pwSrc;
                pwSrc    -= bytesPerRow;
            }

            pwDst += dstMarginWidth;
            pwSrc += srcMarginWidth + 1;
        }
#else
        dirtyRectDump_core_ASM(pwSrc, pwDst, rowLen, srcWidthB, bytesPerRow, srcMarginWidth, dstMarginWidth);
#endif
    }
}

void DispDrvrDirtyRectDump(LPCRECT prc)
{
    BYTE  * pDstBuf;
    BYTE  * pSrcBuf;
    DWORD   xLeft;
    DWORD   yTop;
    DWORD   xRight;
    DWORD   yBottom;
    DWORD   bytesPerRow;
    DWORD   bytesPerPixel;
    DWORD   srcWidthB;
    DWORD   srcMarginWidth;
    DWORD   dstMarginWidth;
    DWORD   srcStartRow;
    DWORD   dstStartRow;

    bytesPerPixel = bpp / 8;

    xLeft   = prc->left   < 0                    ? 0                 : prc->left;
    yTop    = prc->top    < 0                    ? 0                 : prc->top;
    xRight  = prc->right  > DispDrvr_cxScreen    ? DispDrvr_cxScreen : prc->right;
    yBottom = prc->bottom > DispDrvr_cyScreen    ? DispDrvr_cyScreen : prc->bottom;

    if ((LONG)xLeft >= (LONG)xRight || (LONG)yTop >= (LONG)yBottom)
    {
        return;
    }

    xLeft       *= bytesPerPixel;
    xRight      *= bytesPerPixel;
    bytesPerRow  = DispDrvr_cxScreen * bytesPerPixel;

    srcWidthB      = xRight - xLeft;
    srcStartRow    = (yBottom - 1) * bytesPerRow;
    srcMarginWidth = (yBottom - yTop) * bytesPerRow;
    dstStartRow    = xLeft * DispDrvr_cyScreen;
    dstMarginWidth = (DispDrvr_cyScreen - (yBottom - yTop)) * bytesPerPixel;
    pDstBuf        = gFrameBuffer + dstStartRow + (DispDrvr_cyScreen - yBottom) * bytesPerPixel;

    pSrcBuf = gDibBuffer + srcStartRow + xLeft;

    EnterCriticalSection(&displayMutex);

    DirtyRectDumpPortraitLoop_C(pDstBuf, pSrcBuf, yTop, yBottom, srcWidthB, bytesPerRow,
                                bytesPerPixel, srcMarginWidth, dstMarginWidth);

    LeaveCriticalSection(&displayMutex);
}

void DirtyRectDumpPortraitLoop_C_rectfill(BYTE    *pDstBuf, WORD srcColor,DWORD yTop,DWORD yBottom,
     DWORD srcWidthB, DWORD bytesPerRow, DWORD bytesPerPixel, DWORD srcMarginWidth, DWORD dstMarginWidth)
{
    DWORD   row;
    DWORD   i;
    WORD  * pwDst;
    WORD    rowLen;

    //16-bit
    srcWidthB >>= 1;
    pwDst       = (WORD *)pDstBuf;

    //first row for pwSrc, then column for pwDst
    rowLen = (WORD)(yBottom - yTop);

    bytesPerRow    >>= 1;
    dstMarginWidth >>= 1;
    srcMarginWidth >>= 1;

    for (i = 0; i < srcWidthB; i++)
    {
        for (row = 0; row < (DWORD)(rowLen >> 2); row++)
        {
            *pwDst++ = srcColor;
            *pwDst++ = srcColor;
            *pwDst++ = srcColor;
            *pwDst++ = srcColor;
        }

        for (row = 0; row < (DWORD)(rowLen & 0x3); row++)
        {
            *pwDst++ = srcColor;
        }

        pwDst += dstMarginWidth;
    }
}

void DispDrvrDirtyRectDump_rectfill(LPCRECT prc, DWORD color)
{
    BYTE  * pDstBuf;
    BYTE  * pSrcBuf;
    WORD    srcColor = (WORD)color;
    DWORD   xLeft;
    DWORD   yTop;
    DWORD   xRight;
    DWORD   yBottom;
    DWORD   bytesPerRow;
    DWORD   bytesPerPixel;
    DWORD   srcWidthB;
    DWORD   srcMarginWidth;
    DWORD   dstMarginWidth;
    DWORD   srcStartRow;
    DWORD   dstStartRow;
    DWORD   srcMarginWidth2;
    DWORD   dstMarginWidth2;
    DWORD   dstStep;

    bytesPerPixel = bpp / 8;

    xLeft   = prc->left   < 0                    ? 0                 : prc->left;
    yTop    = prc->top    < 0                    ? 0                 : prc->top;
    xRight  = prc->right  > DispDrvr_cxScreen    ? DispDrvr_cxScreen : prc->right;
    yBottom = prc->bottom > DispDrvr_cyScreen    ? DispDrvr_cyScreen : prc->bottom;

    if ((LONG)xLeft >= (LONG)xRight || (LONG)yTop >= (LONG)yBottom)
    {
        return;
    }

    xLeft       *= bytesPerPixel;
    xRight      *= bytesPerPixel;
    bytesPerRow  = DispDrvr_cxScreen * bytesPerPixel;

    srcWidthB       = xRight - xLeft;
    srcStartRow     = (yBottom - 1) * bytesPerRow;
    srcMarginWidth  = (yBottom - yTop) * bytesPerRow;
    dstStartRow     = xLeft * DispDrvr_cyScreen;
    dstMarginWidth  = (DispDrvr_cyScreen - (yBottom - yTop)) * bytesPerPixel;
    pDstBuf         = gFrameBuffer + dstStartRow + (DispDrvr_cyScreen -yBottom) * bytesPerPixel;
    dstStep         = DispDrvr_cyScreen * bytesPerPixel; //portrait frame buffer step
    srcMarginWidth2 = DispDrvr_cxScreen * bytesPerPixel + (xRight - xLeft);
    dstMarginWidth2 = (xRight - xLeft) * dstStep / bytesPerPixel;
    pSrcBuf         = gDibBuffer + srcStartRow + xLeft;

    EnterCriticalSection(&displayMutex);

    DirtyRectDumpPortraitLoop_C_rectfill(pDstBuf, srcColor, yTop, yBottom, srcWidthB, bytesPerRow,
        bytesPerPixel, srcMarginWidth, dstMarginWidth);

    LeaveCriticalSection(&displayMutex);
}

void DispDrvrDirtyRectDump2(LPCRECT prc,DWORD color)
{
    WORD  * pDstBuf;
    WORD    srcColor = (WORD)color;
    DWORD   xLeft;
    DWORD   yTop;
    DWORD   xRight;
    DWORD   yBottom;
    DWORD   srcWidthB;
    DWORD   dstMarginWidth;
    DWORD   dstStartRow;

    xLeft   = prc->left   < 0                    ? 0                 : prc->left;
    yTop    = prc->top    < 0                    ? 0                 : prc->top;
    xRight  = prc->right  > DispDrvr_cxScreen    ? DispDrvr_cxScreen : prc->right;
    yBottom = prc->bottom > DispDrvr_cyScreen    ? DispDrvr_cyScreen : prc->bottom;

    if ((LONG)xLeft >= (LONG)xRight || (LONG)yTop >= (LONG)yBottom)
    {
        return;
    }

    srcWidthB      = xRight - xLeft;
    dstStartRow    = xLeft * DispDrvr_cyScreen;
    dstMarginWidth = (DispDrvr_cyScreen - (yBottom - yTop));
    pDstBuf        = (WORD *)gFrameBuffer + dstStartRow + (DispDrvr_cyScreen - yBottom);

    EnterCriticalSection(&displayMutex);

    ellipse_core_ASM(srcColor, (dstMarginWidth + 1) * 2, srcWidthB, pDstBuf);

    LeaveCriticalSection(&displayMutex);
}

void DispDrvrMoveCursor(INT32 xLocation,INT32 yLocation)
{
    // will be rewritten to take advantage of the new hardware cursor support
/*
    // First clear the cursor's old location
    gDrawCursorFlag = FALSE;
    DispDrvrDirtyRectDump(&gCursorRect);
    // Now set the new location of the cursor and redraw
    gCursorRect.left = xLocation - gxHot;
    if (gCursorRect.left < 0) {
        gCursorRect.left = 0;
    }
    gCursorRect.top = yLocation - gyHot;
    if (gCursorRect.top < 0) {
        gCursorRect.top = 0;
    }
    gCursorRect.right = xLocation - gxHot + CURSOR_XSIZE;
    gCursorRect.bottom = yLocation - gyHot + CURSOR_YSIZE;
    gDrawCursorFlag = TRUE;
    DispDrvrDirtyRectDump(&gCursorRect);
*/
}

#define VIDEO_REG_PATH     TEXT("Drivers\\Display\\PXA27x\\Config")
#define VIDEO_ROW_RES      TEXT("CxScreen")
#define VIDEO_COL_RES      TEXT("CyScreen")
#define PIXEL_DEPTH        TEXT("Bpp")
#define VIDEO_DISPLAY_TYPE TEXT("DisplayType")
#define VIDEO_DISABLE_SCREENROTATION TEXT("DisableScreenRotation")
#define VIDEO_ENABLE_SCREENBLANKING TEXT("EnableScreenBlanking")

BOOL ReadRegistryData()
{
    LONG  regError;
    HKEY  hKey;
    DWORD dwDataSize;
    TCHAR DisplayType[64];

    bpp               = 0;
    DispDrvr_cyScreen = 0;
    DispDrvr_cxScreen = 0;

    // Open the registry key
    regError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,VIDEO_REG_PATH,0,KEY_ALL_ACCESS,&hKey);
    if (regError != ERROR_SUCCESS)
    {
        DEBUGMSG(DEBUGZONE(0),(VIDEO_REG_PATH));
        DEBUGMSG(DEBUGZONE(0),(TEXT("Failed opening \\Drivers\\Display\\PXA27x\\Config\r\n")));
        return (FALSE);
    }

    // Display width
    dwDataSize = sizeof(DispDrvr_cxScreen);
    regError   = RegQueryValueEx(hKey,VIDEO_ROW_RES, NULL, NULL,(LPBYTE)&DispDrvr_cxScreen,&dwDataSize);
    if (regError != ERROR_SUCCESS)
    {
        DEBUGMSG(DEBUGZONE(0),(TEXT("Failed to get display x value, Error 0x%X\r\n"),regError));
        return (FALSE);
    }

    // Display height
    dwDataSize = sizeof(DispDrvr_cyScreen);
    regError   = RegQueryValueEx(hKey,VIDEO_COL_RES,NULL,NULL,(LPBYTE)&DispDrvr_cyScreen,&dwDataSize);
    if (regError != ERROR_SUCCESS) {
        DEBUGMSG(DEBUGZONE(0),(TEXT("Failed to get display y value, Error 0x%X\r\n"),regError));
        return (FALSE);
    }

    // Color depth
    dwDataSize = sizeof(bpp);
    regError=RegQueryValueEx(hKey,PIXEL_DEPTH,NULL,NULL,(LPBYTE)&bpp,&dwDataSize);
    if (regError != ERROR_SUCCESS)
    {
        bpp = 0;
    }

    // Display Type
    dwDataSize = sizeof(DisplayType);
    regError   = RegQueryValueEx(hKey,VIDEO_DISPLAY_TYPE,NULL,NULL,(LPBYTE)DisplayType,&dwDataSize);
    if (regError != ERROR_SUCCESS)
    {
        DEBUGMSG(DEBUGZONE(0),(TEXT("Failed to get display type, Error 0x%X\r\n"),regError));
        return (FALSE);
    }

    // Allow screen rotation?
    dwDataSize = sizeof(g_fDisableRotation);
    regError=RegQueryValueEx(hKey,VIDEO_DISABLE_SCREENROTATION,NULL,NULL,(LPBYTE)&g_fDisableRotation,&dwDataSize);
    if (regError != ERROR_SUCCESS)
    {
        g_fDisableRotation = FALSE;
    }

    // Enable forced screen blacking (for LCD's that ghost)
    dwDataSize = sizeof(g_fEnableDMASourceSwap);
    regError=RegQueryValueEx(hKey,VIDEO_ENABLE_SCREENBLANKING,NULL,NULL,(LPBYTE)&g_fEnableDMASourceSwap,&dwDataSize);
    if (regError != ERROR_SUCCESS)
    {
        g_fEnableDMASourceSwap = FALSE;
    }

    RegCloseKey (hKey);
    RETAILMSG(1, (TEXT("Done getting Registry values:\r\nbpp: 0x%x\r\n CxScreen: 0x%x\r\n CyScreen: 0x%x\r\nDisplay Type: %s\r\n"), bpp, DispDrvr_cxScreen, DispDrvr_cyScreen,DisplayType));

    if (_wcsicmp(DisplayType, TEXT("LTM04C380K")) == 0)
    {
        nDisplayType = LTM04C380K;
    }
    else if (_wcsicmp(DisplayType, TEXT("LM8V31")) == 0)
    {
        nDisplayType = LM8V31;
    }
    else if (_wcsicmp(DisplayType, TEXT("LQ64D341")) == 0)
    {
        nDisplayType = LQ64D341;
    }
    else if (_wcsicmp(DisplayType, TEXT("LTM035A776C")) == 0)
    {
        nDisplayType = LTM035A776C;
    }
    else
    {
        nDisplayType = NONE;
    }

    // bDoRotation is used to indicate whether or not a rotation of the frame buffer
    // is required in order to orient it correctly for the target display.
    bDoRotation = FALSE;
    switch (nDisplayType)
    {
    case LTM04C380K:    // native landscape 640x480
    case LM8V31:        // native landscape 640x480
        if (DispDrvr_cxScreen < DispDrvr_cyScreen)
        {
            bDoRotation = TRUE;
        }
        break;

    case LQ64D341:        // native portrait, 176x220
    case LTM035A776C:    // native portrait, 240x320
        if (DispDrvr_cxScreen > DispDrvr_cyScreen)
        {
            bDoRotation = TRUE;
        }
        break;

    default:
        break;
    }

    // Calculate the stride of the frame buffer
    DispDrvr_cdwStride = DispDrvr_cxScreen * bpp / 8;

    return (TRUE);
}

void ScrollBuffer(int direction)
{
    EnterCriticalSection(&frameDescriptorMutex);

    // Set the physical address of the frame buffer for all three frame descriptors
    if (direction == 1) // scroll up
    {
        frameDescriptorCh0fd1->FSADR += DispDrvr_cdwStride << 2;
        frameDescriptorCh0fd2->FSADR += DispDrvr_cdwStride << 2;
        frameDescriptorCh1->FSADR    += DispDrvr_cdwStride << 2;
    }
    else // scroll down
    {
        frameDescriptorCh0fd1->FSADR -= DispDrvr_cdwStride << 2;
        frameDescriptorCh0fd2->FSADR -= DispDrvr_cdwStride << 2;
        frameDescriptorCh1->FSADR    -= DispDrvr_cdwStride << 2;
    }

    LeaveCriticalSection(&frameDescriptorMutex);
}


