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

//
// KD Ioctl routines
//

#include <windows.h>
#include <oal.h>

//Bits for R/Wn fields in DR7
#define DR7_WRITE_ONLY      0x1
#define DR7_READ_OR_WRITE   0x3
#define DR7_READ_WRITE_MASK 0x3

//+++
// Hardware Debug Register support 
//
__inline void CLEARDR0()
{
    __asm 
    { 
        push eax
        xor eax,eax
        mov dr0, eax
        mov eax, dr7
        and eax,0x0FFFFFFC;
        mov dr7, eax
        pop eax
    }
}

__inline void CLEARDR1()
{
    __asm 
    { 
        push eax
        xor eax,eax
        mov dr1, eax
        mov eax, dr7
        and eax,0xF0FFFFF3;
        mov dr7, eax
        pop eax
    }
}

__inline void CLEARDR2()
{
    __asm 
    { 
        push eax
        xor eax,eax
        mov dr2, eax
        mov eax, dr7
        and eax,0xFF0FFFCF;
        mov dr7, eax
        pop eax
    }
}

__inline void CLEARDR3()
{
    __asm 
    { 
        push eax
        xor eax,eax
        mov dr3, eax
        mov eax, dr7
        and eax,0xFFF0FF3F;
        mov dr7, eax
        pop eax
    }
}

ULONG ReadDR0()
{
    ULONG ulDR0;
    __asm
    {
        mov eax, dr0
        mov ulDR0, eax    
    }    
    return ulDR0;
}

void WriteDR0(ULONG ulValue)
{
    __asm
    {
        mov eax, ulValue
        mov dr0, eax  
    }    
}

ULONG ReadDR1()
{
    ULONG ulDR1;
    __asm
    {
        mov eax, dr1
        mov ulDR1, eax    
    }    
    return ulDR1;
}

void WriteDR1(ULONG ulValue)
{
    __asm
    {
        mov eax, ulValue
        mov dr1, eax  
    }    
}

ULONG ReadDR2()
{
    ULONG ulDR2;
    __asm
    {
        mov eax, dr2
        mov ulDR2, eax    
    }    
    return ulDR2;
}

void WriteDR2(ULONG ulValue)
{
    __asm
    {
        mov eax, ulValue
        mov dr2, eax  
    }    
}

ULONG ReadDR3()
{
    ULONG ulDR3;
    __asm
    {
        mov eax, dr3
        mov ulDR3, eax    
    }    
    return ulDR3;
}

void WriteDR3(ULONG ulValue)
{
    __asm
    {
        mov eax, ulValue
        mov dr3, eax  
    }    
}

ULONG ReadDR6()
{
    ULONG ulDR6;
    __asm
    {
        mov eax, dr6
        mov ulDR6, eax    
    }    
    return ulDR6;
}

void WriteDR6(ULONG ulValue)
{
    __asm
    {
        mov eax, ulValue
        mov dr6, eax  
    }    
}

ULONG ReadDR7()
{
    ULONG ulDR7;
    __asm
    {
        mov eax, dr7
        mov ulDR7, eax    
    }    
    return ulDR7;
}

void WriteDR7(ULONG ulValue)
{
    __asm
    {
        mov eax, ulValue
        mov dr7, eax  
    }    
}


BOOL OEMKDIoControl( DWORD dwIoControlCode, LPVOID lpBuf, DWORD nBufSize)
{
    UNREFERENCED_PARAMETER(nBufSize);
    switch (dwIoControlCode) {
    case KD_IOCTL_INIT:
        CLEARDR0();
        CLEARDR1();
        CLEARDR2();
        CLEARDR3();
        return TRUE;
    case KD_IOCTL_SET_CBP:
    case KD_IOCTL_CLEAR_CBP:
    case KD_IOCTL_ENUM_CBP:
        break;
    case KD_IOCTL_QUERY_CBP:
        ((PKD_BPINFO)lpBuf)->ulCount = 0;
        return TRUE;
    case KD_IOCTL_SET_DBP:
        {   
            PKD_BPINFO pKDBPI = (PKD_BPINFO)lpBuf;
            ULONG ulAddress = pKDBPI->ulAddress;
            ULONG ulReadWriteFlags = (pKDBPI->ulFlags & KD_HBP_FLAG_READ ? DR7_READ_OR_WRITE : DR7_WRITE_ONLY);  
            ULONG ulDR7 = 0;
            
            pKDBPI->ulHandle = 0;

            if(ReadDR3() == 0)
            {
                WriteDR3(ulAddress);
                ulDR7 = ReadDR7();
                ulDR7 |= 0xC00003C0 | (ulReadWriteFlags << 28);
                WriteDR7(ulDR7);        
                pKDBPI->ulHandle = 4;
            }
            else if(ReadDR2() == 0)
            {
                WriteDR2(ulAddress);        
                ulDR7 = ReadDR7();
                ulDR7 |= 0x0C000330 | (ulReadWriteFlags << 24);
                WriteDR7(ulDR7);        
                pKDBPI->ulHandle = 3;
            }
            else if(ReadDR1() == 0)
            {

                WriteDR1(ulAddress); 
                ulDR7 = ReadDR7();
                ulDR7 |= 0x00C0030C | (ulReadWriteFlags << 20);
                WriteDR7(ulDR7);            
                pKDBPI->ulHandle = 2;    
            }
            else if(ReadDR0() == 0)
            {
                WriteDR0(ulAddress);   
                ulDR7 = ReadDR7();
                ulDR7 |= 0x000C0303 | (ulReadWriteFlags << 16);
                WriteDR7(ulDR7);                    
                pKDBPI->ulHandle = 1;        
            }                       
            else
            {
                return FALSE;
            }
            return TRUE;
        }
        break;
    case KD_IOCTL_CLEAR_DBP:
        if (((PKD_BPINFO)lpBuf)->ulHandle == 4) {
            CLEARDR3();
            return TRUE;
        }
        if (((PKD_BPINFO)lpBuf)->ulHandle == 3) {
            CLEARDR2();
            return TRUE;
        }
        if (((PKD_BPINFO)lpBuf)->ulHandle == 2) {
            CLEARDR1();
            return TRUE;
        }
        if (((PKD_BPINFO)lpBuf)->ulHandle == 1) {
            CLEARDR0();
            return TRUE;
        }
        break;
    case KD_IOCTL_QUERY_DBP:
        ((PKD_BPINFO)lpBuf)->ulCount = 4;
        return TRUE;
    case KD_IOCTL_ENUM_DBP:
    case KD_IOCTL_MAP_EXCEPTION:
        {
            //Determine which watch point was hit and fill out ulAddress and ulFlags in  the exception info struct.
            PKD_EXCEPTION_INFO pkdei = (PKD_EXCEPTION_INFO) lpBuf;
            ULONG ulDR7 = ReadDR7();
            ULONG ulRW = 0;
            ULONG ulMask = 0xf;  //Mask for b0-b3 in DSR
            ULONG ulDSR = 0;
            ULONG ulClearDSR = 0;
            
            pkdei->ulFlags = 0;
            pkdei->ulAddress = 0;

            //Read and clear DR6 (DSR)
            ulDSR = ReadDR6();
            ulClearDSR = ulDSR & ~ulMask;
            WriteDR6(ulClearDSR);

            ulDSR &= ulMask;
            switch(ulDSR)
            {
                case 0x1:
                    pkdei->ulAddress = ReadDR0();
                    ulRW = (ulDR7 >> 16) & DR7_READ_WRITE_MASK;
                break;
                case 0x2:
                    pkdei->ulAddress = ReadDR1();
                    ulRW = (ulDR7 >> 20) & DR7_READ_WRITE_MASK;
                break;
                case 0x4:
                    pkdei->ulAddress = ReadDR2();
                    ulRW = (ulDR7 >> 24) & DR7_READ_WRITE_MASK;
                break;
                case 0x8:
                    pkdei->ulAddress = ReadDR3();
                    ulRW = (ulDR7 >> 28) & DR7_READ_WRITE_MASK;
                break;
                default:
                    return FALSE;
            }

            switch(ulRW)
            {
                case DR7_WRITE_ONLY:
                    pkdei->ulFlags = KD_HBP_FLAG_WRITE;
                break;
                case DR7_READ_OR_WRITE:
                    pkdei->ulFlags = KD_HBP_FLAG_READ | KD_HBP_FLAG_WRITE;
                break;
                default:
                    return FALSE;
            }
            
            return TRUE;
        }                         
        break;      
    case KD_IOCTL_RESET:
    default:
        break;
    }
    return FALSE;
}

