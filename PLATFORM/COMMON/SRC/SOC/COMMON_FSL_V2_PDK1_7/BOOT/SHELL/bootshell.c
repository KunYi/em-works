//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//
//------------------------------------------------------------------------------
//
//  Copyright (C) 2008, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  bootshell.c
//

//------------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4115 4201 4214)
#include <windows.h>
#include <ceddk.h>
#include <nkintr.h>
#include <oal.h>
#include <winnls.h>
#pragma warning(pop)
#include "bootshell.h"
//------------------------------------------------------------------------------

BOOL ShowRegFlag,SetRegFlag,SetBitFlag,ExitingShellFlag;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Function:  OutputHelpInformation
//
//  Provides .
//
//  Parameters:
//      None.
//
//  Returns:
//      
//
//------------------------------------------------------------------------------

VOID OutputHelpInformation()
{
    KITLOutputDebugString("\r\n----------------------------- Help -------------------------------\r\n");    
    KITLOutputDebugString("?                                                 Help\r\n");//50 characters before Help;
    KITLOutputDebugString("e                                                 Exit Shell\r\n");
    KITLOutputDebugString("d RegAddress                                      Show Reg\r\n");
    KITLOutputDebugString("s RegAddress RegValue                             Set Reg\r\n");
    KITLOutputDebugString("b RegAddress BitOffset(0-31) Value(0 or 1)        Set Bit\r\n");
    KITLOutputDebugString("------------------------------ End --------------------------------\r\n");    
    KITLOutputDebugString("\r\nAll the input parameters are separated by space key!\r\n");
}

//------------------------------------------------------------------------------
//
//  Function:  SetReg
//
//  Provides .
//
//  Parameters:
//    
//
//  Returns:
//      
//
//------------------------------------------------------------------------------
BOOL SetReg(UCHAR * AddrSet,UCHAR * ValueSet)
{
    UINT32 Addr_num;
    UINT32 Value_num;
    UINT32 *VirtualAddr;
    
    Addr_num = HexString2Hex(AddrSet);
    //KITLOutputDebugString("Address = 0x%x\r\n",Addr_num);

    VirtualAddr = (UINT32 *)OALPAtoUA(Addr_num);
    if (VirtualAddr == NULL) 
    {
        KITLOutputDebugString("Invalid Address,Please Input Again !\r\n");
        return FALSE;
    }

    Value_num = HexString2Hex(ValueSet);
    // KITLOutputDebugString("Value_num = 0x%x\r\n",Value_num);

    *VirtualAddr = Value_num;
    SetRegFlag = FALSE;
    Value_num = *VirtualAddr;
    KITLOutputDebugString("New Info :  Reg Addr = 0x%x,Value = 0x%x \r\n",Addr_num,Value_num);

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  ShowReg
//
//  Provides .
//
//  Parameters:
//    
//
//  Returns:
//      
//
//------------------------------------------------------------------------------
BOOL ShowReg(UCHAR * AddrSet)
{
    UINT32 Addr_num;
    UINT32 Value_num;
    UINT32 *VirtualAddr;

    Addr_num = HexString2Hex(AddrSet);
    // KITLOutputDebugString("Address = 0x%x\r\n",Addr_num);

    VirtualAddr = (UINT32 *)OALPAtoUA(Addr_num);
    if (VirtualAddr == NULL) 
    {
        KITLOutputDebugString("Invalid Address,Please Input Again !\r\n");
        return FALSE;
    }

    else 
    {
        Value_num = *VirtualAddr;
        KITLOutputDebugString("Physical Reg Addr = %s,Value = 0x%x \r\n",AddrSet,Value_num);
        ShowRegFlag = FALSE;
    }

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  SetBit
//
//  Provides .
//
//  Parameters:
//    
//
//  Returns:
//      
//
//------------------------------------------------------------------------------
BOOL SetBit(UCHAR * AddrSet,UCHAR * BitSet,UCHAR * ValueSet)
{
    UINT32 Addr_num;
    UINT32 Value_num;
    UINT32 Bit_num;
    UINT32 *VirtualAddr;

    Addr_num = HexString2Hex(AddrSet);
    // KITLOutputDebugString("Address = 0x%x\r\n",Addr_num);

    VirtualAddr = (UINT32 *)OALPAtoUA(Addr_num);
    if (VirtualAddr == NULL) 
    {
        KITLOutputDebugString("Invalid Address,Please Input Again !\r\n");
        return FALSE;
    }

    Bit_num = OffsetString2Decimal(BitSet[0],BitSet[1]);
    // KITLOutputDebugString("Bit  = 0x%x\r\n",Bit_num); 
    if (Bit_num > 31 || Bit_num < 0) 
    {
        KITLOutputDebugString("Invalid Bit index,Please Input Again !\r\n");
        return FALSE;
    }
    
    if(ValueSet[0]=='0')
        *VirtualAddr = (*VirtualAddr) & (~(0x1<<Bit_num));
    else
        *VirtualAddr = (*VirtualAddr) | ((0x1<<Bit_num));
    
    SetBitFlag = FALSE;
    Value_num = *VirtualAddr;
    KITLOutputDebugString("New Info : Reg Addr = 0x%x,Value = 0x%x \r\n",Addr_num,Value_num);

    return TRUE;
}

//------------------------------------------------------------------------------
//
//  Function:  HexString2Hex
//
//  Provides .
//
//  Parameters:
//    
//
//  Returns:
//      
//
//------------------------------------------------------------------------------
UINT32 HexString2Hex(UCHAR * str)
{
    UINT32 num=0;
    INT8 i=0,k=0,len=0;

    for(k=9;k>=0;k--) 
    {
        if((str[k]<='9'&&str[k]>='0')||(str[k]<='f'&&str[k]>='a')||(str[k]<='F'&&str[k]>='A'))
            len++;
        if(str[k]=='x'||str[k]=='X')
            break;
    }

    while(i<len) 
    {
        switch(str[i+2]) 
        {
            case '0':
                num += 0;
                break;
            case '1':
                num += 1<<((len-1-i)*4);
                break;
            case '2':
                num += 2<<((len-1-i)*4);
                break;
            case '3':
                num += 3<<((len-1-i)*4);
                break;
            case '4':
                num += 4<<((len-1-i)*4);
                break;        
            case '5':
                num += 5<<((len-1-i)*4);
                break;
            case '6': 
                num += 6<<((len-1-i)*4);
                break;
            case '7':
                num += 7<<((len-1-i)*4);
                break;
            case '8': 
                num += 8<<((len-1-i)*4);
                break;
            case '9': 
                num += 9<<((len-1-i)*4);
                break;
            case 'a':
            case 'A':    
                num += 10<<((len-1-i)*4);
                break;
            case 'b':
            case 'B':   
                num += 11<<((len-1-i)*4);
                break;
            case 'c':
            case 'C':     
                num += 12<<((len-1-i)*4);
                break;
            case 'd':
            case 'D':    
                num += 13<<((len-1-i)*4);
                break;
            case 'e':
            case 'E':   
                num += 14<<((len-1-i)*4);
                break;
            case 'f':
            case 'F':    
                num += 15<<((len-1-i)*4);
                break;
            default :
                break;
        }
        i++;
    }
    return num;
}

//------------------------------------------------------------------------------
//
//  Function:  OffsetString2Decimal
//
//  Provides .
//
//  Parameters:
//    
//
//  Returns:
//      
//
//------------------------------------------------------------------------------
UINT32 OffsetString2Decimal(UCHAR str1,UCHAR str2)
{
    if (str2>'0'&&str2<='9')
        return ((str1-0x30)*10 + (str2 -0x30));
    else
     return (str1 -0x30);
}


//------------------------------------------------------------------------------
//
//  Function:  DebugGetLine
//
//  Provides .
//
//  Parameters:
//     
//
//  Returns:
//      
//
//------------------------------------------------------------------------------
VOID DebugGetLine(UCHAR *Linebuffer)
{
    UINT LinebufferPointer;
    UCHAR Keystroke;
    BOOL EndOfLine;

    LinebufferPointer = 0;
    EndOfLine = FALSE;
    while(EndOfLine == FALSE) 
    {
        Keystroke = OEMWaitAndReadDebugByte();
      // Always let user erase last character
        if(Keystroke == 0x08) 
        {
            OEMWriteDebugByte(Keystroke);
            OEMWriteDebugByte(' '); // first erase character from screen
            if(LinebufferPointer > 0) 
            {
                LinebufferPointer --;
                OEMWriteDebugByte(0x08); // then back up cursor again
            }
        }
      // If user isn't at end
        else if(LinebufferPointer < 80) 
        {
            OEMWriteDebugByte(Keystroke);
            if(Keystroke == 0x0D) 
            {
                Linebuffer[LinebufferPointer++] = 0;
                OEMWriteDebugByte(0x0A);
                EndOfLine = TRUE;
            }
            else 
            {
                Linebuffer[LinebufferPointer++] = Keystroke;
            }
        }
   }
}

//------------------------------------------------------------------------------
//
//  Function:  OEMWaitAndReadDebugByte
//
//  Provides .
//
//  Parameters:
//     
//
//  Returns:
//      
//
//------------------------------------------------------------------------------
UCHAR OEMWaitAndReadDebugByte()
{
    int ch;
    while((ch = OEMReadDebugByte()) == OEM_DEBUG_READ_NODATA);
    return((UCHAR)ch);
}



