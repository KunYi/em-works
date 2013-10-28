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
#include <bootDisplayBios.h>
#include <bootBios.h>
#include <bootMemory.h>
#include <bootLog.h>

//------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct VbeInfoBlock_t {
    uint32_t signature;
    uint16_t version;
    uint16_t oemStringPtr[2];
    uint8_t  capabilities[4];
    uint16_t videoModesPtr[2];
    uint16_t totalMemory;
    uint16_t oemSoftwareRev;
    uint16_t oemVendorNamePtr[2];
    uint16_t oemProductNamePtr[2];
    uint16_t oemProductRevPtr[2];
    uint8_t reserved[222];
    uint8_t oemData[256];
} VbeInfoBlock_t;

typedef struct VbeModeInfoBlock_t {
    uint16_t modeAttributes;            // 0000
    uint8_t  winAAttributes;            // 0002
    uint8_t  winBAttributes;            // 0003
    uint16_t winGranularity;            // 0004
    uint16_t winSize;                   // 0006
    uint16_t winASegment;               // 0008
    uint16_t winBSegment;               // 000A
    uint16_t winFuncPrt[2];             // 000C
    uint16_t bytesPerScanLine;          // 0010
    
    uint16_t xResolution;               // 0012
    uint16_t yResolution;               // 0014
    uint8_t  xCharSize;                 // 0016
    uint8_t  yCharSize;                 // 0017
    uint8_t  numberOfPlanes;            // 0018
    uint8_t  bitsPerPixel;              // 0019
    uint8_t  numberOfBanks;             // 001A
    uint8_t  memoryModel;               // 001B
    uint8_t  bankSize;                  // 001C
    uint8_t  numberOfImagePages;        // 001D
    uint8_t  reserved1;                 // 001E

    uint8_t  redMaskSize;               // 001F
    uint8_t  redFieldPosition;          // 0020
    uint8_t  greenMaskSize;             // 0021
    uint8_t  greenFieldPosition;        // 0022
    uint8_t  blueMaskSize;              // 0023
    uint8_t  blueFieldPosition;         // 0024
    uint8_t  rsvdMaskSize;              // 0025
    uint8_t  rsvdFieldPosition;         // 0026
    uint8_t  directColorModeInfo;       // 0027

    uint32_t physBasePtr;               // 0028
    uint16_t reserved2[3];              // 002C

    uint16_t linBytesPerScanLine;       // 0032
    uint8_t  bnkNumberOfImagePages;     // 0033
    uint8_t  linNumberOfImagePages;     // 0034
    uint8_t  linRedMaskSize;            // 0035
    uint8_t  linRedFieldPosition;       // 0036
    uint8_t  linGreenMaskSize;          // 0037
    uint8_t  linGreenFieldPosition;     // 0038
    uint8_t  linBlueMaskSize;           // 0039
    uint8_t  linBlueFieldPosition;      // 003A
    uint8_t  linRsvdMaskSize;           // 003B
    uint8_t  linRsvdFieldPosition;      // 003C
    uint8_t  maxPixelClock;             // 003D
    
    uint8_t  reserved3[189];            // 002C
} VbeModeInfoBlock_t;

typedef struct VbeCrtcInfoBlock_t {
    uint16_t horizontalTotal;
    uint16_t horizontalSyncStart;
    uint16_t horizontalSyncEnd;
    uint16_t verticalTotal;
    uint16_t verticalSyncStart;
    uint16_t verticalSyncEnd;
    uint8_t  flags;
    uint32_t pixelClock;
    uint16_t refreshRate;
    uint8_t  reserved[40];
} VbeCrtcInfoBlock_t;

#pragma pack(pop)

//------------------------------------------------------------------------------

typedef struct DisplayMode_t {
    uint16_t vbeMode; 
    size_t width;
    size_t height;
    size_t bpp;
    uint32_t phFrame;
    size_t stride;
    enum_t redSize;
    enum_t redPos;
    enum_t greenSize;
    enum_t greenPos;
    enum_t blueSize;
    enum_t bluePos;
} DisplayMode_t;

typedef struct Display_t {
    BootDriverVTable_t *pVTable;
    enum_t mode;
    enum_t modes;
    DisplayMode_t *aMode;
    uint16_t *pFrame;
} Display_t;

//------------------------------------------------------------------------------

bool_t
BootDisplayBiosDeinit(
    void *pContext
    );

bool_t
BootDisplayBiosIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    );

//------------------------------------------------------------------------------

static
BootDriverVTable_t
s_displayVTable = {
    BootDisplayBiosDeinit,
    BootDisplayBiosIoCtl
};

static
Display_t
s_display;

//------------------------------------------------------------------------------

static
bool_t
DisplayModeQuery(
    Display_t *pDisplay,
    enum_t *pMode,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pBpp,
    uint32_t *pVesaMode,
    uint32_t *pPhFrame,
    uint32_t *pStride,
    uint32_t *pRedSize,
    uint32_t *pRedPos,
    uint32_t *pGreenSize,
    uint32_t *pGreenPos,
    uint32_t *pBlueSize,
    uint32_t *pBluePos
    );

static
bool_t
DisplayModeSet(
    Display_t *pDisplay,
    enum_t mode
    );

static
bool_t
DisplayFillRect(
    Display_t *pDisplay,
    RECT *pRect,
    uint32_t color
    );

static
bool_t
DisplayBltRect(
    Display_t *pDisplay,
    RECT *pRect,
    void *pBuffer
    );

static
bool_t
DisplaySleep(
    Display_t *pDisplay
    );

static
bool_t
DisplayAwake(
    Display_t *pDisplay
    );

//------------------------------------------------------------------------------

handle_t
BootDisplayBiosInit(
    )
{
    void *pContext = NULL;
    uint32_t eax, ebx, ecx, edx, esi, edi;
    VbeInfoBlock_t *pInfo = BootBiosBuffer();
    VbeModeInfoBlock_t *pModeInfo = (VbeModeInfoBlock_t *)&pInfo[1];
    uint16_t *pModes;
    enum_t modes, idx;
    bool_t vesa3;


    // Create store context
    memset(&s_display, 0, sizeof(Display_t));
    s_display.pVTable = &s_displayVTable;

    // Find if there is VBE support (VESA)
    pInfo = BootBiosBuffer();
    memset(pInfo, 0, sizeof(*pInfo));
    pInfo->signature = '2EBV';
    eax = 0x4F00;
    edi = (uint16_t)((uint32_t)pInfo);
    BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if (((eax & 0xFFFF) != 0x004F) || (pInfo->version < 0x0200))
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosInit: "
            L"BIOS doesn't support VESA 2.0+ Extension!\r\n"
            ));
        goto cleanUp;
        }
    vesa3 = pInfo->version >= 0x0300;

    // Look how many video modes are usefull
    pModes = BootBiosPtr2Void(pInfo->videoModesPtr);

    idx = modes = 0;
    while (pModes[idx] != 0xFFFF)
        {
        eax = 0x4F01;
        ecx = pModes[idx++];
        edi = (uint32_t)pModeInfo;
        BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
        if ((eax & 0xFFFF) != 0x004F) continue;
        // We need at least 176 x 220
        if (pModeInfo->xResolution < 176) continue;
        if (pModeInfo->yResolution < 220) continue;
        // Linear frame buffer must be avaiable
        if ((pModeInfo->modeAttributes & (1 << 7)) == 0) continue;
        // Do not support 15bpp configs
        if (pModeInfo->bitsPerPixel == 15) continue;
        // Support only RGB 5/6/5 mode for 16bpp configs
        if (pModeInfo->bitsPerPixel == 16)
            {
            if (vesa3)
                {
                if (pModeInfo->linRedMaskSize != 5) continue;
                if (pModeInfo->linGreenMaskSize != 6) continue;
                if (pModeInfo->linBlueMaskSize != 5) continue;
                }
            else
                {
                if (pModeInfo->redMaskSize != 5) continue;
                if (pModeInfo->greenMaskSize != 6) continue;
                if (pModeInfo->blueMaskSize != 5) continue;
                }
            }
        // One more video mode we can use
        modes++;        
        }
    if (modes == 0)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosInit: "
            L"No usable VESA mode find!\r\n"
            ));
        goto cleanUp;
        }

    // Allocate structure to save display mode info
    s_display.modes = modes;
    s_display.aMode = BootAlloc(sizeof(DisplayMode_t) * modes);
    if (s_display.aMode == NULL)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosInit: "
            L"Memory allocation failed!\r\n"
            ));
        goto cleanUp;
        }

    // Save mode information
    pModes = BootBiosPtr2Void(pInfo->videoModesPtr);
    idx = modes = 0;
    while (pModes[idx] != 0xFFFF)
        {
        DisplayMode_t *pMode = &s_display.aMode[modes];
        
        eax = 0x4F01;
        ecx = pModes[idx++];
        edi = (uint32_t)pModeInfo;
        BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
        if ((eax & 0xFFFF) != 0x004F) continue;
        // We need at least 176 x 220
        if (pModeInfo->xResolution < 176) continue;
        if (pModeInfo->yResolution < 220) continue;
        // Linear frame buffer must be avaiable
        if ((pModeInfo->modeAttributes & (1 << 7)) == 0) continue;
        // Do not support 15bpp configs
        if (pModeInfo->bitsPerPixel == 15) continue;
        // Support only RGB 5/6/5 mode for 16bpp configs
        if (pModeInfo->bitsPerPixel == 16)
            {
            if (vesa3)
                {
                if (pModeInfo->linRedMaskSize != 5) continue;
                if (pModeInfo->linGreenMaskSize != 6) continue;
                if (pModeInfo->linBlueMaskSize != 5) continue;
                }
            else
                {
                if (pModeInfo->redMaskSize != 5) continue;
                if (pModeInfo->greenMaskSize != 6) continue;
                if (pModeInfo->blueMaskSize != 5) continue;
                }
            }
        // Save info
        pMode->vbeMode = pModes[idx - 1];
        pMode->width = pModeInfo->xResolution;
        pMode->height = pModeInfo->yResolution;
        pMode->bpp = pModeInfo->bitsPerPixel;
        pMode->phFrame = pModeInfo->physBasePtr;
        if (vesa3)
            {
            pMode->redSize = pModeInfo->linRedMaskSize;
            pMode->redPos = pModeInfo->linRedFieldPosition;
            pMode->greenSize = pModeInfo->linGreenMaskSize;
            pMode->greenPos = pModeInfo->linGreenFieldPosition;
            pMode->blueSize = pModeInfo->linBlueMaskSize;
            pMode->bluePos = pModeInfo->linBlueFieldPosition;
            pMode->stride = pModeInfo->linBytesPerScanLine;
            }
        else
            {
            pMode->redSize = pModeInfo->redMaskSize;
            pMode->redPos = pModeInfo->redFieldPosition;
            pMode->greenSize = pModeInfo->greenMaskSize;
            pMode->greenPos = pModeInfo->greenFieldPosition;
            pMode->blueSize = pModeInfo->blueMaskSize;
            pMode->bluePos = pModeInfo->blueFieldPosition;
            pMode->stride = pModeInfo->bytesPerScanLine;
            }

        // Move to next mode
        modes++;        
        }
    
    s_display.mode = (enum_t)-1;
    
    // Done
    pContext = &s_display;

cleanUp:
    if (pContext == NULL) BootDisplayBiosDeinit(&s_display);
    return pContext;
}

//------------------------------------------------------------------------------

bool_t
BootDisplayBiosDeinit(
    void *pContext
    )
{
    bool_t rc = false;
    Display_t *pDisplay = pContext;


    // Check display handle
    if (pDisplay != &s_display)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosDeinit: "
            L"Invalid display handle!\r\n"
            ));
        goto cleanUp;
        }

    // Release allocated mode table
    BootFree(pDisplay->aMode);
    
    // Delete context (oops, clear it only)
    memset(pDisplay, 0, sizeof(Display_t));

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

bool_t
BootDisplayBiosIoCtl(
    void *pContext,
    enum_t code,
    void *pBuffer,
    size_t size
    )
{
    bool_t rc = false;
    Display_t *pDisplay = pContext;


    // Check display handle
    if (pDisplay != &s_display)
        {
        BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosIoCtl: "
            L"Invalid display handle!\r\n"
            ));
        goto cleanUp;
        }

    switch (code)
        {
        case BOOT_DISPLAY_IOCTL_FILLRECT:
            {
            BootDisplayFillRectParams_t* pParams = pBuffer;

            // Check input parameter
            if ((pBuffer == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosIoCtl: "
                    L"BOOT_Display_t_IOCTL_FILLRECT - Invalid parameters!\r\n"
                    ));
                break;
                }
            rc = DisplayFillRect(pDisplay, pParams->pRect, pParams->color);
            }
            break;
        case BOOT_DISPLAY_IOCTL_BLTRECT:
            {
            BootDisplayBltRectParams_t* pParams;

            // Check input parameter
            if ((pBuffer == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosIoCtl: "
                    L"BOOT_DISPLAY_IOCTL_BLTRECT - Invalid parameters!\r\n"
                    ));
                break;
                }
            pParams = (BootDisplayBltRectParams_t*)pBuffer;
            rc = DisplayBltRect(pDisplay, pParams->pRect, pParams->pBuffer);
            }
            break;
        case BOOT_DISPLAY_IOCTL_SLEEP:
            rc = DisplaySleep(pDisplay);
            break;
        case BOOT_DISPLAY_IOCTL_AWAKE:
            rc = DisplayAwake(pDisplay);
            break;
        case BOOT_DISPLAY_IOCTL_MODE_QUERY:
            {
            BootDisplayModeQueryParams_t *pParams = pBuffer;
        
            // Check input parameter
            if ((pBuffer == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosIoCtl: "
                    L"BOOT_DISPLAY_IOCTL_MODE_QUERY - Invalid parameters!\r\n"
                    ));
                break;
                }
            rc = DisplayModeQuery(
                pDisplay, &pParams->mode, &pParams->width, &pParams->height,
                &pParams->bpp, &pParams->vesaMode, &pParams->phFrame,
                &pParams->stride, &pParams->redSize, &pParams->redPos,
                &pParams->greenSize, &pParams->greenPos, &pParams->blueSize,
                &pParams->bluePos
                );
            }
            break;
        case BOOT_DISPLAY_IOCTL_MODE_SET:
            {
            BootDisplayModeSetParams_t *pParams = pBuffer;
        
            // Check input parameter
            if ((pBuffer == NULL) || (size != sizeof(*pParams)))
                {
                BOOTMSG(ZONE_ERROR, (L"ERROR: BootDisplayBiosIoCtl: "
                    L"BOOT_DISPLAY_IOCTL_MODE_SET - Invalid parameters!\r\n"
                    ));
                break;
                }
            rc = DisplayModeSet(pDisplay, pParams->mode);
            }
            break;
        }

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
DisplayModeQuery(
    Display_t *pDisplay,
    enum_t *pMode,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pBpp,
    uint32_t *pVesaMode,
    uint32_t *pPhFrame,
    uint32_t *pStride,
    uint32_t *pRedSize,
    uint32_t *pRedPos,
    uint32_t *pGreenSize,
    uint32_t *pGreenPos,
    uint32_t *pBlueSize,
    uint32_t *pBluePos
    )
{
    bool_t rc = false;
    enum_t mode = (enum_t)-1;
    DisplayMode_t *pModeInfo;
    
    if (*pMode == -1)
        {
        if ((*pWidth != 0) || (*pHeight != 0))
            {
            enum_t bestMode = (enum_t)-1;
            uint32_t bestDelta = (uint32_t)-1;
        
            for (mode = 0; mode < pDisplay->modes; mode++)
                {
                uint32_t delta;
                pModeInfo = &pDisplay->aMode[mode];

                // Not best mode if it is less than 16 bpp
                if (pModeInfo->bpp < 16) continue;
                if (*pWidth > pModeInfo->width) continue;
                if (*pHeight > pModeInfo->height) continue;
                delta  = pModeInfo->width - *pWidth;
                delta += pModeInfo->height - *pHeight;
                if (delta < bestDelta)
                    {
                    bestDelta = delta;
                    bestMode = mode;
                    }
                if (bestDelta == 0) break;
                }

            // If we didn't find reasonable mode, fail call...
            if (bestMode == -1) goto cleanUp;

            // This is our mode
            mode = bestMode;
            
            }
        else 
            {
            uint32_t eax, ebx, ecx, edx, esi, edi;

            // Get actual VESA mode            
            eax = 0x4F03;
            BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
            if ((eax & 0xFFFF) != 0x004F) goto cleanUp;
            ebx &= 0x3FFF;

            // Look if it is one from supported        
            for (mode = 0; mode < pDisplay->modes; mode++)
                {
                pModeInfo = &pDisplay->aMode[mode];
                if (pModeInfo->vbeMode == ebx) break;
                }

            // Fail if we don't support it...
            if (mode >= pDisplay->modes) goto cleanUp;
            }
        }
    else if (*pMode < pDisplay->modes)
        {
        mode = *pMode;
        }
    else
        {
        goto cleanUp;
        }

    // Results
    pModeInfo = &pDisplay->aMode[mode];

    *pMode = mode;
    *pVesaMode = pModeInfo->vbeMode;
    *pWidth = pModeInfo->width;
    *pHeight = pModeInfo->height;
    *pBpp = pModeInfo->bpp;
    *pPhFrame = pModeInfo->phFrame;
    *pStride = pModeInfo->stride;
    *pRedSize = pModeInfo->redSize;
    *pRedPos = pModeInfo->redPos;
    *pGreenSize = pModeInfo->greenSize;
    *pGreenPos = pModeInfo->greenPos;
    *pBlueSize = pModeInfo->blueSize;
    *pBluePos = pModeInfo->bluePos;
        
    // Done
    rc = true;
    
cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
DisplayModeSet(
    Display_t *pDisplay,
    enum_t mode
    )
{
    bool_t rc = false;
    uint32_t eax, ebx, ecx, edx, esi, edi;

    BootBiosBuffer();
    if (mode >= pDisplay->modes) goto cleanUp;

    eax = 0x4F02;
    ebx = pDisplay->aMode[mode].vbeMode | (1 << 14);
    edi = 0;
    BootBiosInt10(&eax, &ebx, &ecx, &edx, &esi, &edi);
    if ((eax & 0xFFFF) != 0x004F) goto cleanUp;
    
    // Set actual mode
    pDisplay->mode = mode;

    // Map frame buffer
    pDisplay->pFrame = BootPAtoUA(pDisplay->aMode[mode].phFrame);

    // Done
    rc = true;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
DisplayFillRect(
    Display_t* pDisplay,
    RECT *pRect,
    uint32_t color
    )
{
    bool_t rc = false;
    volatile uint16_t *pFrame = pDisplay->pFrame;
    DisplayMode_t *pMode;
    long xt, yt;

    if (pDisplay->mode >= pDisplay->modes) goto cleanUp;
    pMode = &pDisplay->aMode[pDisplay->mode];

    if (pRect->right < pRect->left) goto cleanUp;
    if (pRect->top > pRect->bottom) goto cleanUp;

    for (yt = pRect->top; yt < pRect->bottom; yt++)
        {
        for (xt = pRect->left; xt < pRect->right; xt++)
            {
            pFrame[yt * (pMode->stride >> 1) + xt] = (uint16_t)color;
            }
        }

    // Done
    rc = true;

cleanUp:    
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
DisplayBltRect(
    Display_t* pDisplay,
    RECT* pRect,
    void *pBuffer
    )
{
    bool_t rc = false;
    volatile uint16_t *pFrame = pDisplay->pFrame;
    DisplayMode_t *pMode;
    uint16_t *pData = (uint16_t*)pBuffer;
    long stride, pos, xt, yt;

    if (pDisplay->mode >= pDisplay->modes) goto cleanUp;
    pMode = &pDisplay->aMode[pDisplay->mode];

    if (pRect->right < pRect->left) goto cleanUp;
    if (pRect->top > pRect->bottom) goto cleanUp;
    
    // Convert stride from bytes to words
    stride = pRect->right - pRect->left;
    pos = (pRect->bottom - pRect->top) * stride;
    for (yt = pRect->top; yt < pRect->bottom; yt++)
        {
        for (xt = pRect->right - 1; xt >= pRect->left; xt--)
            {
            pFrame[yt * (pMode->stride >> 1) + xt] = pData[--pos];
            }
        }

    // Done
    rc = true;

cleanUp:
    return rc;
}

//------------------------------------------------------------------------------

static
bool_t
DisplaySleep(
    Display_t* pDisplay
    )
{
    UNREFERENCED_PARAMETER(pDisplay);
    // Done
    return true;
}

//------------------------------------------------------------------------------

static
bool_t
DisplayAwake(
    Display_t *pDisplay
    )
{
    UNREFERENCED_PARAMETER(pDisplay);
    // Done
    return true;
}

//------------------------------------------------------------------------------

