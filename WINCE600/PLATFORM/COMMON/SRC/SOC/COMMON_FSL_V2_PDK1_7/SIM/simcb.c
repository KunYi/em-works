//------------------------------------------------------------------------------
//
// Copyright (C) 2006-2009 Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  File:  simcb.c
//
//   This file implements the call back functions for sim
//
//------------------------------------------------------------------------------


// Include
#pragma warning(push)
#pragma warning(disable: 4115)
#pragma warning(disable: 4201)
#pragma warning(disable: 4204)
#pragma warning(disable: 4214)
#include <windows.h>
#pragma warning(pop)
#include <devload.h>
#pragma warning(push)
#pragma warning(disable: 4201)
#pragma warning(disable: 4214)
#include <smclib.h>
#include <winsmcrd.h>
#pragma warning(pop)
#include "simcb.h"
#include "simhw.h"

//------------------------------------------------------------------------------
// External Functions


//------------------------------------------------------------------------------
// External Variables


//------------------------------------------------------------------------------
// Defines


//------------------------------------------------------------------------------
// Types


//------------------------------------------------------------------------------
// Global Variables


//------------------------------------------------------------------------------
// Local Variables


//------------------------------------------------------------------------------
// Local Functions

static NTSTATUS CBT0Transmit(PSMARTCARD_EXTENSION SmartcardExtension);
static NTSTATUS T0_ExchangeData(
    PREADER_EXTENSION    ReaderExtension,
    PUCHAR                pRequest,
    ULONG                RequestLen,
    PUCHAR                pReply,
    PULONG                pReplyLen);
static NTSTATUS CBSynchronizeSIM(PSMARTCARD_EXTENSION SmartcardExtension );


//-----------------------------------------------------------------------------
//
// Function: CBCardPower
//
// This function is the callback handler for SMCLIB RDF_CARD_POWER
//
// Parameters:
//      SmartcardExtension
//          [in]Pointer of the context of call
//
// Returns:
//      NTSTATUS:
//      STATUS_SUCCESS
//      STATUS_NO_MEDIA
//      STATUS_TIMEOUT
//      STATUS_BUFFER_TOO_SMALL
//
//-----------------------------------------------------------------------------
NTSTATUS CBCardPower(PSMARTCARD_EXTENSION SmartcardExtension)
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    UCHAR ATRBuffer[ ATR_SIZE ];
    ULONG Command, ATRLength = 0;
    PCSP_SIM_REG pSIMReg;

#if DBG || DEBUG
    static PTCHAR request[] = { TEXT("PowerDown"),  TEXT("ColdReset"), TEXT("WarmReset") };
#endif

    SmartcardDebug(DEBUG_TRACE, ( TEXT("+CBCardPower, Request = %s\n"),request[SmartcardExtension->MinorIoControlCode]));

    pSIMReg = SmartcardExtension->ReaderExtension->pSIMReg;
    
    //update actual power state
    Command = SmartcardExtension->MinorIoControlCode;

    switch ( Command )
    {
        case SCARD_WARM_RESET:
        //if the card was not powerd, fall thru to cold reset
            if( SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_SWALLOWED )
            {
                NTStatus = SIM_WarmReset(SmartcardExtension->ReaderExtension);
                break;
            }
            
        PREFAST_SUPPRESS( 5411, "Break statement left out intentionally to fall through to cold reset" )
        case SCARD_COLD_RESET:
        //reset the card
            ATRLength = ATR_SIZE;
            NTStatus = SIM_ColdReset(SmartcardExtension->ReaderExtension, ATRBuffer, &ATRLength);
            
            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
            break;

        case SCARD_POWER_DOWN:
            ATRLength = 0;
            NTStatus = SIM_Deactivate(SmartcardExtension->ReaderExtension);

            if(NTStatus == STATUS_SUCCESS)
            {
                SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_PRESENT;
            }
            break;
    }

    //    finish the request
    if( NTStatus == STATUS_SUCCESS )
    {

        //    update all neccessary data if an ATR was received
        if( ATRLength > 2 )
        {
            //    copy ATR to user buffer 
            if( ATRLength <= SmartcardExtension->IoRequest.ReplyBufferLength )
            {
                SysCopyMemory(
                    SmartcardExtension->IoRequest.ReplyBuffer,
                    ATRBuffer,
                    ATRLength);
                *SmartcardExtension->IoRequest.Information = ATRLength;
            }
            else
            {
                NTStatus = STATUS_BUFFER_TOO_SMALL;
            }

            //    copy ATR to card capability buffer
            if( ATRLength <= 64 )
            {    
                SysCopyMemory(
                    SmartcardExtension->CardCapabilities.ATR.Buffer,
                    ATRBuffer,
                    ATRLength);

                SmartcardExtension->CardCapabilities.ATR.Length = ( UCHAR )ATRLength;


                //    let the lib update the card capabilities
                NTStatus = SmartcardUpdateCardCapabilities( SmartcardExtension );

            }
            else
            {
                NTStatus = STATUS_BUFFER_TOO_SMALL;
            }

        }
    }


    SmartcardDebug( DEBUG_TRACE,( TEXT("-CBCardPower Exit: %X\n"), NTStatus ));
    return( NTStatus );
}


//-----------------------------------------------------------------------------
//
// Function: CBSetProtocol
//
// This function is the callback handler for SMCLIB RDF_SET_PROTOCOL
//
// Parameters:
//      SmartcardExtension
//          [in]Pointer of the context of call
//
// Returns:
//      NTSTATUS:
//      STATUS_SUCCESS
//      STATUS_NO_MEDIA
//      STATUS_TIMEOUT
//      STATUS_BUFFER_TOO_SMALL
//      STATUS_INVALID_DEVICE_STATE
//      STATUS_INVALID_DEVICE_REQUEST
//
//-----------------------------------------------------------------------------
NTSTATUS CBSetProtocol(PSMARTCARD_EXTENSION SmartcardExtension)
{
    NTSTATUS            NTStatus = STATUS_PENDING;
    UCHAR                PTSRequest[64],
                        PTSReply[64];
    ULONG                NewProtocol;

    SmartcardDebug(DEBUG_TRACE, (TEXT("+CBSetProtocol Enter\n")));

    NewProtocol        = SmartcardExtension->MinorIoControlCode;

    //    check if the card is already in specific state
    if( ( SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC )  &&
        ( SmartcardExtension->CardCapabilities.Protocol.Selected & NewProtocol ))
    {
        NTStatus = STATUS_SUCCESS;    
    }

    //    protocol supported?
    if( !( SmartcardExtension->CardCapabilities.Protocol.Supported & NewProtocol ) || 
        !( SmartcardExtension->ReaderCapabilities.SupportedProtocols & NewProtocol ))
    {
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;    
    }

    //    send PTS
    while( NTStatus == STATUS_PENDING )
    {
        //flush the SIM fifo
        SIM_FlushFIFO(SmartcardExtension->ReaderExtension);

        // set initial character of PTS
        PTSRequest[0] = 0xFF;

        // set the format character
        if(( NewProtocol & SCARD_PROTOCOL_T1 )&&
            (SmartcardExtension->CardCapabilities.Protocol.Supported & SCARD_PROTOCOL_T1 ))
        {
            PTSRequest[1] = 0x11;
            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T1;
        }
        else
        {
            PTSRequest[1] = 0x10;
            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;
        }

        //    PTS1 codes Fl and Dl
        PTSRequest[2] = 
            SmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
            SmartcardExtension->CardCapabilities.PtsData.Dl;

        //    check character
        PTSRequest[3] = PTSRequest[0] ^ PTSRequest[1] ^ PTSRequest[2];   

        //    write PTSRequest
        NTStatus = SIM_WriteData(SmartcardExtension->ReaderExtension, PTSRequest, 4);

        //    get response
        if( NTStatus == STATUS_SUCCESS )
        {
            NTStatus = SIM_ReadData(SmartcardExtension->ReaderExtension, PTSReply, 2 );

            if( PTSReply[1] == 0)
                NTStatus = SIM_ReadData(SmartcardExtension->ReaderExtension, (PTSReply + 2), 1 );
            else
                NTStatus = SIM_ReadData(SmartcardExtension->ReaderExtension, (PTSReply + 2), 2 );

            if(( NTStatus == STATUS_SUCCESS ) && !SysCompareMemory( PTSRequest, PTSReply, 4))
            {
                //    set the SIM registers
                SmartcardExtension->CardCapabilities.Dl =
                    SmartcardExtension->CardCapabilities.PtsData.Dl;
                SmartcardExtension->CardCapabilities.Fl = 
                    SmartcardExtension->CardCapabilities.PtsData.Fl;

                NTStatus = CBSynchronizeSIM( SmartcardExtension );

                // the card replied correctly to the PTS-request
                break;
            }
        }

        //
        //    The card did either NOT reply or it replied incorrectly
        //    so try default values
        //
        if( SmartcardExtension->CardCapabilities.PtsData.Type != PTS_TYPE_DEFAULT )
        {
            SmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_DEFAULT;
            SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
            NTStatus = CBCardPower( SmartcardExtension );

            if( NTStatus == STATUS_SUCCESS )
            {
                NTStatus = STATUS_PENDING;
            }
            else
            {
                NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
            }
        }
    }

    if( NTStatus == STATUS_TIMEOUT )
    {
        NTStatus = STATUS_IO_TIMEOUT;             
    }

    if( NTStatus == STATUS_SUCCESS )
    {
        //    indicate that the card is in specific mode 
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;

        // return the selected protocol to the caller
        *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = SmartcardExtension->CardCapabilities.Protocol.Selected;
        *SmartcardExtension->IoRequest.Information = sizeof(SmartcardExtension->CardCapabilities.Protocol.Selected);
    }
    else
    {
        SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
        *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = 0;
        *SmartcardExtension->IoRequest.Information = 0;
    }

    SmartcardDebug( DEBUG_TRACE, (TEXT("-CBSetProtocol: Exit %X\n"), NTStatus ));

    return( NTStatus ); 
}


//-----------------------------------------------------------------------------
//
// Function: CBTransmit
//
// This function is the callback handler for SMCLIB RDF_TRANSMIT
//
// Parameters:
//      SmartcardExtension
//          [in]Pointer of the context of call
//
// Returns:
//      NTSTATUS:
//      STATUS_SUCCESS
//      STATUS_NO_MEDIA
//      STATUS_TIMEOUT
//      STATUS_INVALID_DEVICE_REQUEST
//
//-----------------------------------------------------------------------------
 NTSTATUS CBTransmit(PSMARTCARD_EXTENSION SmartcardExtension)
{
    NTSTATUS  NTStatus = STATUS_SUCCESS;

    SmartcardDebug( DEBUG_TRACE, (TEXT("+CBTransmit Enter\n")));

    //    dispatch on the selected protocol
    switch( SmartcardExtension->CardCapabilities.Protocol.Selected )
    {
        case SCARD_PROTOCOL_T0:
            NTStatus = CBT0Transmit( SmartcardExtension );
            break;

        case SCARD_PROTOCOL_T1:
            SmartcardDebug(DEBUG_ERROR,( TEXT("T1 protocol is not supported in this driver!\n")));
            return (STATUS_UNSUCCESSFUL);
            break;

        case SCARD_PROTOCOL_RAW:
            SmartcardDebug(DEBUG_ERROR,( TEXT("RAW protocol is not supported in this driver!\n")));
            return (STATUS_UNSUCCESSFUL);
            break;

        default:
            NTStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    SmartcardDebug( DEBUG_TRACE, (TEXT("-CBTransmit Exit: %X\n"),NTStatus ));

    return( NTStatus );
}


//-----------------------------------------------------------------------------
//
// Function: CBT0Transmit
//
// This function finishes the callback RDF_TRANSMIT for the T0 protocol
//
// Parameters:
//      SmartcardExtension
//          [in]Pointer of the context of call
//
// Returns:
//      NTSTATUS:
//      STATUS_SUCCESS
//      STATUS_NO_MEDIA
//      STATUS_TIMEOUT
//      STATUS_INVALID_DEVICE_REQUEST
//
//-----------------------------------------------------------------------------
static NTSTATUS CBT0Transmit(PSMARTCARD_EXTENSION SmartcardExtension)
{
    NTSTATUS NTStatus = STATUS_SUCCESS;

    SmartcardDebug( DEBUG_TRACE, (TEXT("+CBT0Transmit Enter\n")));

    SmartcardExtension->SmartcardRequest.BufferLength = 0;

    //    let the lib setup the T=1 APDU & check for errors
    NTStatus = SmartcardT0Request( SmartcardExtension );

    if( NTStatus == STATUS_SUCCESS )
    {
        NTStatus = T0_ExchangeData(
            SmartcardExtension->ReaderExtension,
            SmartcardExtension->SmartcardRequest.Buffer,
            SmartcardExtension->SmartcardRequest.BufferLength,
            SmartcardExtension->SmartcardReply.Buffer,
            &SmartcardExtension->SmartcardReply.BufferLength);

        if( NTStatus == STATUS_SUCCESS )
        {
            //    let the lib evaluate the result & tansfer the data
            NTStatus = SmartcardT0Reply( SmartcardExtension );
        }
    }

    SmartcardDebug( DEBUG_TRACE,(TEXT("-CBT0Transmit Exit: %X\n"),NTStatus ));

    return( NTStatus );
}


//-----------------------------------------------------------------------------
//
// Function: T0_ExchangeData
//
// This function does T=0 management
//
// Parameters:
//      ReaderExtension
//          [in]Pointer of the context of call
//      pRequest
//          [in]Request buffer
//      RequestLen
//          [in]/[out]Request buffer length
//      pReply
//          [in]Reply buffer
//      pReplyLen
//          [in]/[out]Reply buffer length
//
// Returns:
//      NTSTATUS:
//
//-----------------------------------------------------------------------------
static NTSTATUS T0_ExchangeData(
    PREADER_EXTENSION    ReaderExtension,
    PUCHAR                pRequest,
    ULONG                RequestLen,
    PUCHAR                pReply,
    PULONG                pReplyLen)
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    BOOLEAN Direction;
    UCHAR Ins, Pcb;
    ULONG Len, DataIdx;

    SmartcardDebug( DEBUG_TRACE, (TEXT("+T0_ExchangeData Enter\n")));


    //    get direction
    Ins = pRequest[ INS_IDX ] & 0xFE;
    Len    = pRequest[ P3_IDX ];

    if( RequestLen == 5 )
    {
        Direction    = ISO_OUT;
        DataIdx        = 0;
        // For an ISO OUT command Len=0 means that the host expect an 
        // 256 byte answer
        if( !Len )
        {
            Len = 0x100;
        }
        // Add 2 for SW1 SW2
        Len+=2;

        DataIdx += Len;
    } 
    else
    {
        Direction    = ISO_IN;
        DataIdx        = 5;
    }

    //    send header CLASS,INS,P1,P2,P3
    NTStatus = SIM_WriteData( ReaderExtension, pRequest, 5 );

    if( NTStatus == STATUS_SUCCESS )
    {
        NTStatus = STATUS_MORE_PROCESSING_REQUIRED;
    }

    while( NTStatus == STATUS_MORE_PROCESSING_REQUIRED )
    {
        // PCB reading
        Pcb = 0;
        NTStatus = SIM_ReadData( ReaderExtension, &Pcb, 1 );
        
        if( NTStatus == STATUS_SUCCESS )
        {
            if( Pcb == 0x60 )
            {
                //    null byte?
                NTStatus = STATUS_MORE_PROCESSING_REQUIRED;
                continue;
            }
            else if( ( Pcb & 0xFE ) == Ins )
            {
                //    transfer all
                if( Direction == ISO_IN )
                {
                    //    write remaining data
                    NTStatus = SIM_WriteData( ReaderExtension, pRequest + DataIdx, Len );
                    if( NTStatus == STATUS_SUCCESS )
                    {
                        //    if all data successful written the status word is expected
                        NTStatus    = STATUS_MORE_PROCESSING_REQUIRED;
                        Direction    = ISO_OUT;
                        DataIdx        = 0;
                        Len            = 2;
                    }
                }
                else
                {
                    //    read remaining data
                    NTStatus = SIM_ReadData( ReaderExtension, pReply, Len );
                    
                }
            }
            else if( (( Pcb & 0xFE ) ^ Ins ) == 0xFE )
            {
                //    transfer next
                if( Direction == ISO_IN )
                {
                    //    write next

                    NTStatus = SIM_WriteData( ReaderExtension, pRequest + DataIdx, 1 );

                    if( NTStatus == STATUS_SUCCESS )
                    {
                        DataIdx++;

                        //    if all data successful written the status word is expected
                        if( --Len == 0 )
                        {
                            Direction    = ISO_OUT;
                            DataIdx        = 0;
                            Len            = 2;
                        }
                        NTStatus = STATUS_MORE_PROCESSING_REQUIRED;
                    }
                }
                else
                {
                    //    read next
                    NTStatus = SIM_ReadData( ReaderExtension, pReply + DataIdx, 1 );


                    if( NTStatus == STATUS_SUCCESS )
                    {
                        NTStatus = STATUS_MORE_PROCESSING_REQUIRED;
                        Len--;
                        DataIdx++;
                    }
                }
            }
            else if( ((( Pcb & 0x60 ) == 0x60 ) || (( Pcb & 0x90 ) == 0x90 )) && ( Pcb != 0x60 ) )
            {
                if( Direction == ISO_IN )
                {
                    Direction    = ISO_OUT;
                    DataIdx        = 0;
                }

                //    SW1
                *pReply    = Pcb;

                //    read SW2 and leave
                NTStatus = SIM_ReadData( ReaderExtension, &Pcb, 1 );

                *(pReply + 1)    = Pcb;
                DataIdx            += 2;
            }
            else
            {
                NTStatus = STATUS_UNSUCCESSFUL;
            }
        }
    }

    if(( NTStatus == STATUS_SUCCESS ) && ( pReplyLen != NULL ))
    {
        *pReplyLen = DataIdx;
    }

    SmartcardDebug( DEBUG_TRACE, (TEXT("-T0_ExchangeData Enter\n")));

    return( NTStatus );        
}


//-----------------------------------------------------------------------------
//
// Function: CBCardTracking
//
// This function is the callback handler for SMCLIB RDF_CARD_TRACKING
//
// Parameters:
//      SmartcardExtension
//          [in]Pointer of the context of call
//
// Returns:
//      NTSTATUS:
//      STATUS_PENDING
//
//-----------------------------------------------------------------------------
NTSTATUS CBCardTracking(PSMARTCARD_EXTENSION SmartcardExtension)
{
    SmartcardDebug(DEBUG_TRACE, (TEXT("+CBCardTracking\n")));

    SmartcardExtension->ReaderExtension->IoctlPending=TRUE;

    SmartcardDebug(DEBUG_TRACE, (TEXT("-CBCardTracking\n")));

    return( STATUS_PENDING );

}


//-----------------------------------------------------------------------------
//
// Function: CBSynchronizeSIM
//
// This function updates the card dependend data of the SIM
//
// Parameters:
//      SmartcardExtension
//          [in]Pointer of the context of call
//
// Returns:
//      NTSTATUS
//
//-----------------------------------------------------------------------------
static NTSTATUS CBSynchronizeSIM(PSMARTCARD_EXTENSION SmartcardExtension)
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    BOOL Clock, Time;

    SmartcardDebug(DEBUG_TRACE, (TEXT("+CBSynchronizeSIM\n")));
    
    //    set character waiting & guard time
    switch ( SmartcardExtension->CardCapabilities.Protocol.Selected )
    {
        case SCARD_PROTOCOL_T0:
            Clock = SIM_ConfigCLK(SmartcardExtension->ReaderExtension, 
                SmartcardExtension->CardCapabilities.Fl, 
                SmartcardExtension->CardCapabilities.Dl);

            if(Clock == FALSE)
                SmartcardDebug( DEBUG_DRIVER,( TEXT("Problems in clock configuration\n")));

            Time = SIM_ConfigTime(SmartcardExtension->ReaderExtension, 
                SmartcardExtension->CardCapabilities.N, 
                SmartcardExtension->CardCapabilities.T0.WI);
            if(Time == FALSE)
                SmartcardDebug( DEBUG_DRIVER,( TEXT("Problems in time configuration\n")));    
            break;

        case SCARD_PROTOCOL_T1:
                SmartcardDebug(DEBUG_ERROR,( TEXT("T1 protocol is not supported in this driver!\n")));
                
        PREFAST_SUPPRESS( 5411, "Break statement left out intentionally to fall through to default case" )
        default:
            NTStatus = STATUS_UNSUCCESSFUL;
            break;
    }

    SmartcardDebug(DEBUG_TRACE, (TEXT("-CBSynchronizeSIM\n")));

    return( NTStatus );
}



//-----------------------------------------------------------------------------
//
// Function: CBUpdateCardState
//
// This function updates the variable CurrentState in SmartcardExtension
//
// Parameters:
//      SmartcardExtension
//          [in]Pointer of the context of call
//
// Returns:
//      NTSTATUS:
//      STATUS_SUCCESS
//
//-----------------------------------------------------------------------------
NTSTATUS CBUpdateCardState(PSMARTCARD_EXTENSION SmartcardExtension)
{
    NTSTATUS NTStatus = STATUS_SUCCESS;

    SmartcardDebug(DEBUG_TRACE, (TEXT("+CBUpdateCardState\n")));
    
    //    read card state
    SmartcardLockDevice(SmartcardExtension);

    if(SIM_CheckCard(SmartcardExtension->ReaderExtension))
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_PRESENT;
    else
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_ABSENT;

    SmartcardUnlockDevice(SmartcardExtension);
    SmartcardDebug(DEBUG_TRACE, (TEXT("-CBUpdateCardState\n")));
    
    return(NTStatus);
}


//  -------------------------------- END OF FILE ------------------------------




