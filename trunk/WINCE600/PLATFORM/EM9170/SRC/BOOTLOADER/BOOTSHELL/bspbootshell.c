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
//  File:  bspbootshell.c
//
//------------------------------------------------------------------------------
// Include Files
#include <bsp.h>
#include "bootshell.h"


//------------------------------------------------------------------------------
// Local Variaties;
UCHAR Linebuffer[512],BspAddrSet[10],BspBitSet[2],BspValueSet[10];
//------------------------------------------------------------------------------
//Extern Function
extern BOOL SetBit(UCHAR * AddrSet,UCHAR * BitSet,UCHAR * ValueSet);
extern BOOL ShowReg(UCHAR * AddrSet);
extern BOOL SetReg(UCHAR * AddrSet,UCHAR * ValueSet);
extern VOID OutputHelpInformation();
extern VOID DebugGetLine(UCHAR *Linebuffer);
extern UCHAR OEMWaitAndReadDebugByte();


//------------------------------------------------------------------------------

extern BOOL ShowRegFlag,SetRegFlag,SetBitFlag,ExitingShellFlag;

//------------------------------------------------------------------------------
static BOOL ParseCommand(UCHAR * str);
//
//  Function:  BootShell
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

VOID BspBootShell()
{
    UCHAR token;
    
    KITLOutputDebugString("-----------  Freescale iMX Boot Shell  ----------\r\n");
    KITLOutputDebugString("type ? for help\r\n"); 
    ExitingShellFlag = FALSE;
    while( ExitingShellFlag == FALSE ) 
    {
        ExitingShellFlag = FALSE;
        ShowRegFlag = FALSE;
        SetRegFlag = FALSE;
        SetBitFlag = FALSE;
        memset(BspAddrSet,' ',sizeof(BspAddrSet));
        memset(BspBitSet,' ',sizeof(BspBitSet));
        memset(BspValueSet,' ',sizeof(BspValueSet));
        memset(Linebuffer,' ',sizeof(Linebuffer));

        // Get a line of user input
        KITLOutputDebugString(BOOT_MONITOR_PROMPT);
        DebugGetLine(Linebuffer);
        
        //       Extract first token from user input
        //token = extractToken(Linebuffer, " ", NULL);
        token = Linebuffer[0];

        if(ParseCommand(Linebuffer)==FALSE) 
        {
            KITLOutputDebugString("Invalid parameter,Please Input Again !\r\n");
            continue;
        }
        
        switch(token) 
        {
            case '?':
                OutputHelpInformation();
                break;
            case 'd':
            case 'D':
                ShowRegFlag = TRUE;
                break;
            case 'e':
            case 'E':
                ExitingShellFlag = TRUE;
                break;
            case 's':
            case 'S':
                SetRegFlag = TRUE;
                break;
            case 'b':
            case 'B':
                SetBitFlag = TRUE;
                break;
            default:
                KITLOutputDebugString("Unknow command! \r\n");
                break;
        }
        

    
        if(ShowRegFlag == TRUE) 
        {
            if(ShowReg(BspAddrSet) == FALSE) 
            {
                ShowRegFlag = FALSE;
                continue;
            }
        }
    
        if(SetRegFlag == TRUE) 
        {
            if(SetReg(BspAddrSet,BspValueSet) == FALSE) 
            {
                SetRegFlag = FALSE;
                continue;
            }    
        }
        
        if(SetBitFlag == TRUE) 
        {
            if(SetBit(BspAddrSet,BspBitSet,BspValueSet) == FALSE) 
            {
                SetBitFlag = FALSE;
                continue;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function:  ParseCommand
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
BOOL ParseCommand(UCHAR * str)
{
    UINT32 i=2,j=0;
    BOOL ret=TRUE;
    UCHAR type;
    type = str[0];
    switch(type)
    {
        case 's':
     case 'S':
            while(str[i] == ' ')
                i++;
         while(str[i]!=' ') 
                BspAddrSet[j++] = str[i++];
         while(str[i] == ' ')
                i++;
         j=0;
         while(str[i]!=' ') 
            {
                BspValueSet[j++] = str[i++];
            }
            if((BspAddrSet[1]!='x'&&BspAddrSet[1]!='X')||(BspValueSet[1]!='x'&&BspValueSet[1]!='X'))
                ret = FALSE;
         break;
     case 'b':   
     case 'B':   
         while(str[i] == ' ')
                i++;        
         while(str[i]!=' ') 
                BspAddrSet[j++] = str[i++];
         while(str[i] == ' ')
             i++;
         j=0;
         while(str[i]!=' ') 
                BspBitSet[j++] = str[i++];
         while(str[i] == ' ')        
             i++;
         j=0;
         while(str[i]!=' ') 
                BspValueSet[j++] = str[i++];
            if(BspAddrSet[1]!='x'&&BspAddrSet[1]!='X')
                ret = FALSE;
         break;        
     case 'd':   
     case 'D':   
         while(str[i] == ' ')
                i++;       
         while(str[i]!=' ') 
                BspAddrSet[j++] = str[i++];
            if(BspAddrSet[1]!='x'&&BspAddrSet[1]!='X')
                ret = FALSE;
         break;    
     default:
         break;
    }
    return ret;
}



