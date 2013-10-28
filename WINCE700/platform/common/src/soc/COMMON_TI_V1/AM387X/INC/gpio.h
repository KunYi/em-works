/*
================================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
================================================================================
*/
//
//  File:  gpio.h
//
//  This header defines interface for GPIO device driver. This driver control
//  GPIO pins on hardware. It allows abstract GPIO interface and break up
//  physicall and logical pins. To avoid overhead involved the driver exposes
//  interface which allows obtain funtion pointers to base set/clr/get etc.
//  functions.
//
#ifndef __GPIO_H
#define __GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//
//  Define:  GPIO_DEVICE_NAME
//
#define GPIO_DEVICE_NAME        L"GIO1:"

//------------------------------------------------------------------------------
//
//  Define:  GPIO_DIR_xxx/GPIO_INT_xxx
//
#define GPIO_DIR_OUTPUT         (0 << 0)
#define GPIO_DIR_INPUT          (1 << 0)
#define GPIO_INT_LOW_HIGH       (1 << 1)
#define GPIO_INT_HIGH_LOW       (1 << 2)
#define GPIO_INT_LOW            (1 << 3)
#define GPIO_INT_HIGH           (1 << 4)
#define GPIO_DEBOUNCE_ENABLE    (1 << 5)

//------------------------------------------------------------------------------
//
//  GUID:  DEVICE_IFC_GPIO_GUID
//
DEFINE_GUID(
    DEVICE_IFC_GPIO_GUID, 0xa0272611, 0xdea0, 0x4678,
    0xae, 0x62, 0x65, 0x61, 0x5b, 0x7d, 0x53, 0xaa
);

//------------------------------------------------------------------------------
// Enum : Gpio Numbers
//
typedef enum {
    //GPIOBank0
    GPIO_0 = 0,
    GPIO_1,
    GPIO_2,
    GPIO_3,
    GPIO_4,
    GPIO_5,
    GPIO_6,
    GPIO_7,
    GPIO_8,
    GPIO_9,
    GPIO_10,
    GPIO_11,
    GPIO_12,
    GPIO_13,
    GPIO_14,
    GPIO_15,
    GPIO_16,
    GPIO_17,
    GPIO_18,
    GPIO_19,
    GPIO_20,
    GPIO_21,
    GPIO_22,
    GPIO_23,
    GPIO_24,
    GPIO_25,
    GPIO_26,
    GPIO_27,
    GPIO_28,
    GPIO_29,
    GPIO_30,
    GPIO_31,

    //GPIOBank1
    GPIO_32,
    GPIO_33,
    GPIO_34,
    GPIO_35,
    GPIO_36,
    GPIO_37,
    GPIO_38,
    GPIO_39,
    GPIO_40,
    GPIO_41,
    GPIO_42,
    GPIO_43,
    GPIO_44,
    GPIO_45,
    GPIO_46,
    GPIO_47,
    GPIO_48,
    GPIO_49,
    GPIO_50,
    GPIO_51,
    GPIO_52,
    GPIO_53,
    GPIO_54,
    GPIO_55,
    GPIO_56,
    GPIO_57,
    GPIO_58,
    GPIO_59,
    GPIO_60,
    GPIO_61,
    GPIO_62,
    GPIO_63,

    //GPIOBank2
    GPIO_64,
    GPIO_65,
    GPIO_66,
    GPIO_67,
    GPIO_68,
    GPIO_69,
    GPIO_70,
    GPIO_71,
    GPIO_72,
    GPIO_73,
    GPIO_74,
    GPIO_75,
    GPIO_76,
    GPIO_77,
    GPIO_78,
    GPIO_79,
    GPIO_80,
    GPIO_81,
    GPIO_82,
    GPIO_83,
    GPIO_84,
    GPIO_85,
    GPIO_86,
    GPIO_87,
    GPIO_88,
    GPIO_89,
    GPIO_90,
    GPIO_91,
    GPIO_92,
    GPIO_93,
    GPIO_94,
    GPIO_95,
	
    //GPIOBank3
    GPIO_96,
    GPIO_97,
    GPIO_98,
    GPIO_99,
    GPIO_100,
    GPIO_101,
    GPIO_102,
    GPIO_103,
    GPIO_104,
    GPIO_105,
    GPIO_106,
    GPIO_107,
    GPIO_108,
    GPIO_109,
    GPIO_110,
    GPIO_111,
    GPIO_112,
    GPIO_113,
    GPIO_114,
    GPIO_115,
    GPIO_116,
    GPIO_117,
    GPIO_118,
    GPIO_119,
    GPIO_120,
    GPIO_121,
    GPIO_122,
    GPIO_123,
    GPIO_124,
    GPIO_125,
    GPIO_126,
    GPIO_127,

	GPIO_MAX_NUM
} GpioNum_e;

//------------------------------------------------------------------------------
//
//  Type: function prototype
//
//  Predefines a set of function callbacks used to manage gpio's exposed by
//  multiple silicon
//
typedef BOOL (*fnGpioInit)(LPCTSTR szContext, HANDLE *phContext, UINT *pGpioCount);
typedef BOOL (*fnGpioDeinit)(HANDLE hContext);
typedef BOOL (*fnGpioSetMode)(HANDLE hContext, UINT id, UINT mode);
typedef BOOL (*fnGpioGetMode)(HANDLE hContext, UINT id, UINT *pMode);
typedef BOOL (*fnGpioInterruptInitialize)(HANDLE context, UINT intrID,HANDLE hEvent);
typedef BOOL (*fnGpioInterruptRelease)(HANDLE hContext, UINT id);
typedef BOOL (*fnGpioInterruptDone)(HANDLE hContext, UINT id);
typedef BOOL (*fnGpioInterruptDisable)(HANDLE hContext, UINT id);
typedef BOOL (*fnGpioInterruptMask)(HANDLE hContext, UINT id, BOOL bEnable);
typedef BOOL (*fnGpioEnableWake)(HANDLE hContext, UINT id, BOOL bEnable);
typedef BOOL (*fnGpioSetBit)(HANDLE hContext, UINT id);
typedef BOOL (*fnGpioClrBit)(HANDLE hContext, UINT id);
typedef BOOL (*fnGpioGetBit)(HANDLE hContext, UINT id, UINT *pValue);
typedef void (*fnGpioPowerUp)(HANDLE hContext);
typedef void (*fnGpioPowerDown)(HANDLE hContext);
typedef BOOL (*fnGpioIoControl)(HANDLE hContext, UINT code,
                                UCHAR *pinVal, UINT inSize, UCHAR *poutVal,
                                UINT outSize, UINT *pOutSize);

//------------------------------------------------------------------------------
//  Predefines structure containing gpio call back routines exposed for a
//  silicon
typedef struct {
    fnGpioInit      Init;
    fnGpioDeinit    Deinit;
    fnGpioSetMode   SetMode;
    fnGpioGetMode   GetMode;
    fnGpioInterruptInitialize   InterruptInitialize;
    fnGpioInterruptRelease      InterruptRelease;
    fnGpioInterruptDone         InterruptDone;
    fnGpioInterruptDisable      InterruptDisable;
    fnGpioInterruptMask         InterruptMask;
    fnGpioEnableWake            EnableWake;
    fnGpioSetBit    SetBit;
    fnGpioClrBit    ClrBit;
    fnGpioGetBit    GetBit;
    fnGpioPowerUp   PowerUp;
    fnGpioPowerDown PowerDown;
    fnGpioIoControl IoControl;
} GPIO_TABLE;

//------------------------------------------------------------------------------
//  This structure is used to obtain GPIO interface funtion pointers used for
//  in-process calls via IOCTL_DDK_GET_DRIVER_IFC.
typedef struct {
    DWORD context;
    VOID  (*pfnSetBit)(DWORD context, DWORD id);
    VOID  (*pfnClrBit)(DWORD context, DWORD id);
    DWORD (*pfnGetBit)(DWORD context, DWORD id);
    VOID  (*pfnSetMode)(DWORD context, DWORD id, DWORD mode);
    DWORD (*pfnGetMode)(DWORD context, DWORD id);
    DWORD (*pfnInterruptInitialize)(DWORD context, DWORD id, HANDLE hEvent);
    DWORD (*pfnInterruptRelease)(DWORD context, DWORD id);
    DWORD (*pfnInterruptDone)(DWORD context, DWORD id);
    DWORD (*pfnInterruptDisable)(DWORD context, DWORD id);
    DWORD (*pfnInterruptMask)(DWORD context, DWORD id, BOOL bEnable);
    DWORD (*pfnEnableWake)(DWORD context, DWORD id, BOOL bEnable);
} DEVICE_IFC_GPIO;

//------------------------------------------------------------------------------
//  This structure is used to store GPIO device context.
typedef struct {
    DEVICE_IFC_GPIO ifc;
    HANDLE hDevice;
} DEVICE_CONTEXT_GPIO;

//------------------------------------------------------------------------------

#define IOCTL_GPIO_SETBIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0300, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_CLRBIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0301, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_UPDATEBIT    \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0302, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GETBIT       \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0303, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_SETMODE      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0304, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GETMODE      \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0305, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_SET_DEBOUNCE_TIME  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0306, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_GET_DEBOUNCE_TIME  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0307, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_INIT_INTERRUPT  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0308, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_ACK_INTERRUPT  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0309, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_DISABLE_INTERRUPT  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0310, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_RELEASE_INTERRUPT  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0311, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_ENABLE_WAKE  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0312, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Special case to get Hardware Interrupt number for OMAP GPIO pin
#define IOCTL_GPIO_GET_OMAP_HW_INTR  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0313, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_GPIO_MASK_INTERRUPT  \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0314, METHOD_BUFFERED, FILE_ANY_ACCESS)


//------------------------------------------------------------------------------

typedef struct {
    UINT                    gpioId;
    UINT                    debounceTime;
} IOCTL_GPIO_SET_DEBOUNCE_TIME_IN;

typedef struct {
    UINT                    gpioId;
    BOOL                    bEnable;
} IOCTL_GPIO_INTERRUPT_MASK;

typedef struct {
    UINT                    gpioId;
    BOOL                    bEnable;
} IOCTL_GPIO_ENABLE_WAKE_IN;

typedef struct {
    UINT                uGpioID;
    HANDLE              hEvent;
} IOCTL_GPIO_INIT_INTERRUPT_INFO,  *PIOCTL_GPIO_INIT_INTERRUPT_INFO;


//------------------------------------------------------------------------------
__inline HANDLE GPIOOpen()
{
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    DEVICE_CONTEXT_GPIO *pContext = NULL;

    hDevice = CreateFile(GPIO_DEVICE_NAME, 0, 0, NULL, 0, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) goto cleanUp;

    // Allocate memory for our handler...
    pContext = (DEVICE_CONTEXT_GPIO*)LocalAlloc(LPTR, sizeof(DEVICE_CONTEXT_GPIO));
    if (pContext == NULL){
        CloseHandle(hDevice);
        goto cleanUp;
    }

    // Get function pointers.  If not possible (b/c of cross process calls), use IOCTLs instead
    if (!DeviceIoControl(
            hDevice, IOCTL_DDK_GET_DRIVER_IFC, (VOID*)&DEVICE_IFC_GPIO_GUID,
            sizeof(DEVICE_IFC_GPIO_GUID), &pContext->ifc,
            sizeof(DEVICE_IFC_GPIO), NULL, NULL))
        {
        //  Need to use IOCTLs instead of direct function ptrs
        pContext->ifc.context = 0;
        }

    // Save device handle
    pContext->hDevice = hDevice;

cleanUp:
    return pContext;
}

__inline VOID GPIOClose( HANDLE hContext )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;
    if (pContext != NULL){
        CloseHandle(pContext->hDevice);
        LocalFree(pContext);
    }
}

__inline VOID GPIOSetBit( HANDLE hContext, DWORD id )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
        return;

    if( pContext->ifc.context )
        pContext->ifc.pfnSetBit(pContext->ifc.context, id);
    else 
        DeviceIoControl(pContext->hDevice, IOCTL_GPIO_SETBIT, &id, sizeof(id), NULL, 0, NULL, NULL );
}

__inline VOID GPIOClrBit( HANDLE hContext, DWORD id )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
        return;

    if( pContext->ifc.context )
        pContext->ifc.pfnClrBit(pContext->ifc.context, id);
    else
        DeviceIoControl(pContext->hDevice, IOCTL_GPIO_CLRBIT, &id, sizeof(id), NULL, 0, NULL, NULL );
}

__inline DWORD GPIOGetBit( HANDLE hContext, DWORD id )
{
    DWORD   dwValue = 0;
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
        return 0;

    if( pContext->ifc.context )
        return pContext->ifc.pfnGetBit(pContext->ifc.context, id);

    DeviceIoControl(pContext->hDevice, IOCTL_GPIO_GETBIT, &id, sizeof(id),
		            &dwValue, sizeof(dwValue), NULL, NULL );

    return dwValue;
}

__inline VOID GPIOSetMode( HANDLE hContext, DWORD id, DWORD mode )
{
	DWORD   dwValue[2] = {id, mode};
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
        return;

    if( pContext->ifc.context )
        pContext->ifc.pfnSetMode(pContext->ifc.context, id, mode);

    DeviceIoControl(pContext->hDevice, IOCTL_GPIO_SETMODE, &dwValue,
                        sizeof(dwValue), NULL, 0, NULL, NULL );
}

__inline DWORD GPIOGetMode( HANDLE hContext, DWORD id )
{
    DWORD   dwValue = 0;
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if( pContext == NULL )
        return 0;

    if( pContext->ifc.context )
        return pContext->ifc.pfnGetMode(pContext->ifc.context, id);

    DeviceIoControl(pContext->hDevice, IOCTL_GPIO_GETMODE, &id,
                        sizeof(id), &dwValue, sizeof(dwValue), NULL, NULL );

    return dwValue;
}

__inline DWORD GPIOInterruptInitialize( HANDLE hContext, DWORD id, HANDLE hEvent )
{
    DWORD   dwRC;
    IOCTL_GPIO_INIT_INTERRUPT_INFO sInitIntrInfo = {id, hEvent};
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if (hContext == NULL)
        return FALSE;

    if (pContext->ifc.context)
        return pContext->ifc.pfnInterruptInitialize(pContext->ifc.context, id, hEvent);

    dwRC = DeviceIoControl(pContext->hDevice, IOCTL_GPIO_INIT_INTERRUPT, &sInitIntrInfo,
                        sizeof(sInitIntrInfo), NULL, 0, NULL, NULL );

    return dwRC;
}


__inline DWORD GPIOInterruptDone( HANDLE hContext, DWORD id )
{
    DWORD   dwRC;
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if (hContext == NULL)
        return FALSE;

	if (pContext->ifc.context != 0)
        return pContext->ifc.pfnInterruptDone(pContext->ifc.context, id);

    dwRC = DeviceIoControl(pContext->hDevice, IOCTL_GPIO_ACK_INTERRUPT, &id,
                        sizeof(id), NULL, 0, NULL, NULL );

    return dwRC;
}

__inline DWORD GPIOInterruptDisable( HANDLE hContext, DWORD id )
{
    DWORD   dwRC;
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if (hContext == NULL)
        return FALSE;

    if (pContext->ifc.context != 0)
        return pContext->ifc.pfnInterruptDisable(pContext->ifc.context, id);

    dwRC = DeviceIoControl(pContext->hDevice, IOCTL_GPIO_DISABLE_INTERRUPT, &id,
                        sizeof(id), NULL, 0, NULL, NULL );

    return dwRC;
}

__inline DWORD GPIOInterruptMask( HANDLE hContext, DWORD id, BOOL  bEnable )
{
    DWORD   dwRC;
    IOCTL_GPIO_INTERRUPT_MASK   sParam = {id, bEnable};
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if (hContext == NULL)
        return FALSE;

    if (pContext->ifc.context != 0)
        return pContext->ifc.pfnInterruptMask(pContext->ifc.context, id, bEnable);

    dwRC = DeviceIoControl(pContext->hDevice, IOCTL_GPIO_MASK_INTERRUPT, &sParam,
                        sizeof(sParam), NULL, 0, NULL, NULL );

    return dwRC;
}

__inline DWORD GPIOWakeEnable( HANDLE hContext, DWORD id )
{
    DWORD   dwRC;
    IOCTL_GPIO_ENABLE_WAKE_IN   sParam = {id, TRUE};
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if (hContext == NULL)
        return FALSE;

    if (pContext->ifc.context != 0)
        return pContext->ifc.pfnEnableWake(pContext->ifc.context, id, TRUE);

    dwRC = DeviceIoControl(pContext->hDevice, IOCTL_GPIO_ENABLE_WAKE, &sParam,
                        sizeof(sParam), NULL, 0, NULL, NULL );

    return dwRC;
}

__inline DWORD GPIOWakeDisable( HANDLE hContext, DWORD id )
{
    DWORD   dwRC;
	IOCTL_GPIO_ENABLE_WAKE_IN   sParam = {id, FALSE};
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if (hContext == NULL)
        return FALSE;

    if (pContext->ifc.context != 0)
        return pContext->ifc.pfnEnableWake(pContext->ifc.context, id, FALSE);

    dwRC = DeviceIoControl(pContext->hDevice, IOCTL_GPIO_ENABLE_WAKE, &sParam,
                        sizeof(sParam), NULL, 0, NULL, NULL );

    return dwRC;
}

__inline DWORD GPIOInterruptRelease( HANDLE hContext, DWORD id )
{
    DWORD   dwRC;
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if (hContext == NULL)
        return FALSE;

    if (pContext->ifc.context != 0)
        return pContext->ifc.pfnInterruptRelease(pContext->ifc.context, id);

    dwRC = DeviceIoControl(pContext->hDevice, IOCTL_GPIO_RELEASE_INTERRUPT,
                           &id, sizeof(id), NULL, 0, NULL, NULL );

    return dwRC;
}


__inline DWORD GPIOIoControl( HANDLE hContext, DWORD  code, UCHAR *pInBuffer,
    DWORD  inSize, UCHAR *pOutBuffer, DWORD  outSize, DWORD *pOutSize, LPOVERLAPPED pOverlap )
{
    DEVICE_CONTEXT_GPIO *pContext = (DEVICE_CONTEXT_GPIO *)hContext;

    if ( pContext == NULL )
        return 0;

    return DeviceIoControl(pContext->hDevice, code, pInBuffer,
                inSize, pOutBuffer, outSize, pOutSize, pOverlap);
}

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
