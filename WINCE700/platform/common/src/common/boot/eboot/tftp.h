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

  tftp.h

Abstract:

  This contains the tftp.c specific data structure declarations.

Functions:


Notes:


Revision History:

--*/
#ifndef TFTP_H
#define TFTP_H

#include <halether.h>
#include <tftpd.h>

#define MAX_TFTP_LINKS  2                   // Maximum number of TFTP links that can be active at one time
#define TFTP_TX_LINKS   1                   // Number of links that are guaranteed to be available for transmit (O2HLink)
#define DEFAULT_MAX_TFTP_DATA (512+4)           // Maximum number of bytes in a TFTP message (512 bytes plus header)
#define MAX_TFTP_DATA_BUFFERSIZE (1024+4)       //Maximum number of bytes that could ever be in a TFTP message (supporting negotiated block sizes)
#define MAX_RETRIES     4               // Maximum number of times a message will be transmitted due to timeouts


// Data direction of the TFTP link
typedef enum {  H2OLink,    // The link is transfering data from the host to odo
                O2HLink     // The link is transfering data from odo to the host
} TFtpLinkDir;

// These are the possible states for the TFTP links.  They have different meanings
//  depending on whether the link transfers data from odo to the host (O2HLink),
//  or if the link transfers data from the host to odo (H2OLink).
                                        // For H2OLink                          For O2HLink
typedef enum {  TFTP_STATE_IDLE,        // No activity in progress              No activity in progress
                TFTP_STATE_OPEN,        // *State Not Used*                     Write Req sent, waiting for acknowledge
                TFTP_STATE_XFER_BUSY,   // Acknowledge sent, waiting for proc   Data sent, waiting for proc
                TFTP_STATE_XFER_WAIT,   // Acknowledge sent, waiting for data   Data sent, waiting for acknowledge
                TFTP_STATE_CLOSE,       // *State Not Used*                     Last data sent, waiting for acknowledge
                TFTP_STATE_ERROR        // This means that an error occured and the link has been terminated.  If an
                                        //  error occurs, the link will remain in this state until the Odo process
                                        //  driving it can be notified of the error through TFtpRead() or TFtpWrite()
                                        //  for a client or through the callback mechanism for a server process.
} TFtpStates;

typedef enum {  TFTP_OPCODE_UNDEFINED = 0,      //This opcode is un defined
                TFTP_OPCODE_RRQ = 1,                //This is a Read Request opcode
                TFTP_OPCODE_WRQ = 2,                //This is a Write Request opcode
                TFTP_OPCODE_DATA = 3,               //This is a data packet
                TFTP_OPCODE_ACK = 4,                //This is a standard acknowledgement packet
                TFTP_OPCODE_ERROR = 5,          //This is an error packet
                TFTP_OPCODE_OACK = 6                //This is an options acknowledgement packet
} TFtpOpcodes;


// These are the possible error codes that can be passed in a TFTP error packet.  Note that all of them
//  will cause termination of the link they occur on.

typedef enum {  TFTP_ERROR_UNDEFINED = 0,           // Not defined, see error message (if any)
                TFTP_ERROR_FILENOTFOUND = 1,        // File not found
                TFTP_ERROR_ACCESSVIOLATION = 2,     // Access violation
                TFTP_ERROR_DISKFULL = 3,            // Disk full or allocation exceeded
                TFTP_ERROR_ILLEGALOPERATION = 4,    // Illegal TFTP operation
                TFTP_ERROR_UNKNOWNTID = 5,          // Unknown transfer ID
                TFTP_ERROR_FILEEXISTS = 6,          // File already exists
                TFTP_ERROR_NOSUCHUSER = 7,          // No such user
                TFTP_ERROR_HOSTERROR = 8,           // This is an error code that I added to specify that a
                                                    //  host error packet has been received
                TFTP_ERROR_OPTIONSERROR = 9     // I added this error code for erros related to TFTP options
} TFtpErrors;



typedef struct TFtpConnectionRecTag {

    TFtpLinkDir DataDir;            // Contains the direction that data is moving
    TFtpStates State;               // Contains the state that the link is in
    TFtpdCallBackFunc pfCallBack;   // The callback function used if this is used for a server
                                    //  It will be NULL if the link was orginated by us
    char szFileName[MAX_TFTP_FILENAME];
                                    // This is the file name of the data going over the link
    EDBG_ADDR SrcAddr;              // The full address that we are using as the source,
                                    //  including the TID that we are using as a source port
    EDBG_ADDR DestAddr;             // The full address that we are using as the destination,
                                    //  including the TID that we are using as a destination port
    UINT16 wBlockNumber;            // The message block number associated with the Buffer
    BOOL UsedBlkSizeOption;         //Set to 1 if a blksize option was utilized.  This must be recognized with an OACK packet
    UINT16 wMaxTftpData;            //The negotiated size of data blocks for this connection
    UINT16 cwMsgLen;                // Length (in bytes) of the TFTP message in the buffer
    UINT16 cwRetries;               // Number of times this buffer has been transmitted due to retries
    DWORD tLastTransmit;            // Time at which the last transmission of the Buffer info occured
    BYTE Buffer[MAX_TFTP_DATA_BUFFERSIZE];      // The transmit/receive data that is currently being attempted
} TFtpLinkRec;


// These values are returned from the TFtpOpen(), TFtpRead(), TFtpWrite() and TFtpClose() routines.
//  Their meanings are call specific:
                                // Open                 Read                Write
typedef enum {  TFTP_SUCCESS,   // Proceed              Proceed             Proceed
                TFTP_WAIT,      // *Not Returned*       No packets          Waiting for
                                //                      available           acknowledge of
                                //                                          previous packet
                TFTP_ERROR      // Link currently       Connection reset    Connection reset
                                //  in use              or timed out,       or timed out,
                                //                      terminate transfer  terminate transfer
} TFtpReturnCodes;



void InitTFtp( UINT16 wOdoWKSP, UINT16 wHostWKSP );
WORD TFtpReceiver( EDBG_ADDR *pMyAddr, BYTE *pFrameBuffer, UINT16 wDestPort, UINT16 wSrcPort, UINT16 *pwUDPData, UINT16 cwUDPDataLength );
void TFTPRetransmit( void );
void TFtpKillLink( UINT16 iLinkSlot, TFtpErrors ErrorCode, char *pszErrorMessage );
void TFtpStateMachine( WORD wSrcPort, UINT16 iLinkSlot, UINT16 *pwMsg, UINT16 cwMsgLen );
UINT16 TFtpOpen( EDBG_ADDR *pHostAddr, EDBG_ADDR *pMyAddr, char *pszFileName, TFtpLinkDir DataDir );
TFtpReturnCodes TFtpRead( UINT16 wLinkHandle, BYTE *pbData, UINT16 *cwDataLen );
TFtpReturnCodes TFtpWrite( UINT16 wLinkHandle, BYTE *pbData, UINT16 cwDataLen );

// Prototypes from esh.c
BOOL ProcessEDBG(EDBG_ADDR *pMyAddr,BYTE *pFrameBuffer,UINT16 *pwUDPData, UINT16 cwUDPDataLength,UINT16 wDestPort);
void SendBootme(EDBG_ADDR *pMyAddr, DWORD SubnetMask);


extern UINT16 wOdoWellKnownServerPort, wHostWellKnownServerPort;
extern TFtpLinkRec TFtpLinks[MAX_TFTP_LINKS];   // Data records for each of the possible TFTP links
extern UINT16 nNumTFtpLinksInUse;               // The number of active TFTP links



#endif
