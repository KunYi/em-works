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

#ifndef _DEC21140HARDWARE_
#define _DEC21140HARDWARE_

#pragma warning(push)
#pragma warning(disable:4214 4201) 

/////////////////////////////////////////////////////////////////////////////////
//  Misc...
//

#define PAD(label,amt)          UCHAR Pad##label[amt]



/////////////////////////////////////////////////////////////////////////////////
//
//  Others...
//
typedef struct 
{
    DWORD   dwReg;

} CSR_21140;


/////////////////////////////////////////////////////////////////////////////////
//  CSR0 === Bus Mode Register
//

#define     CSR0_MUST_AND   0xFFFeFFFF

#define     SOFTWARE_RESET  0x00000001
#define     LITTLE_ENDIAN   1 << 7

typedef struct tagCSR0
{
    union
    {
        struct
        {
            DWORD   SoftwareReset : 1;              //  [0 ] 1 = Reset all internal h/w except configuration register. 
            DWORD   BusArbitration : 1;             //  [1 ] 1 = Round Robin Arbitration btw tx/rx  ;  0 = Rx higher priority.
            DWORD   DescriptorSkipLength :5;        //  [2 ] Number of longwords to skip between 2 descriptors.
            DWORD   BigLittleEndian : 1;            //  [7 ] 1 = Big Endian.
            DWORD   ProgrammableBurstLength : 6;    //  [8 ] # LongWord transferred in one DMA transaction.   Default 0 --> 16 DW
            DWORD   CacheAlignment : 2;             //  [14] MUST BE INITIALIZE AFTER RESET, Programmable address boundaries for 
                                                    //          data burst stop.

            DWORD   MustBeZero : 1;                 //  [16] Must be 0.
            DWORD   TransmitAutomaticPolling : 3;   //  [17] Set to get 21140A to poll when tx buffer is unavailable.
            DWORD   DescriptorByteOrderingMode : 1; //  [20] 1 = Big Endian ordering mode for Descriptors only.
            DWORD   ReadMultiple : 1;               //  [21] 1 = Read Multiple command on PCI bus.
                        DWORD   Rsvd1 : 1;
                        DWORD   ReadLnEnable : 1;
                        DWORD   WriteInvEnable : 1;
                        DWORD   Rsvd2 : 1;
                        DWORD   OnNow : 1;

            DWORD   ________Reserved : 5;               
        };

        DWORD   dwReg;
    };

} CSR0_21140;


/////////////////////////////////////////////////////////////////////////////////
//  CSR5 === STATUS REGISTER
//

typedef struct tagCSR5
{
    union 
    {
        struct
        {   
            DWORD   TransmitInterrupt : 1;          //  [0 ] Indicates a frame transmission was completed.
            DWORD   TransmitProcessStopped : 1;     //  [1 ] Asserts when transmit process enters the stopped state.
            DWORD   TransmitBufferUnavailable : 1;  //  [2 ] Indicates next descriptor on tx list is owned by host.
            DWORD   TransmitJabberTimeout : 1;      //  [3 ] Causes TDES0[14] flag to assert.

            DWORD   ________Reserved_ : 1;

            DWORD   TransmitUnderflow : 1;          //  [5 ] Causes Underflow error TDES0[1] flag to be set.    
            DWORD   ReceiveInterrupt : 1;           //  [6 ]
            DWORD   ReceiveBufferUnavailable : 1;   //  [7 ]
            DWORD   ReceiveProcessStopped : 1;      //  [8 ]
            DWORD   ReceiveWatchdogTimeout : 1;     //  [9 ]    
            DWORD   EarlyTransmitInterrupt : 1;     //  [10] CSR5[0] automatically clears this bit.
            DWORD   GeneralPurposeTimerExpired : 1; //  [11] CSR11 has expired.

            DWORD   ________Reserved__ : 1;         

            DWORD   FatalBusError : 1;              //  [13] System error, 21140 will then disable all bus accesses.
    
            DWORD   ________Reserved___ : 1;

            DWORD   AbnormalInterruptSummary: 1;    //  [15] Logical OR of CSR5[1, 3, 5, 7, 8, 9, 10, 13]
            DWORD   NormalInterruptSummary: 1;      //  [16] Logical OR of CSR5[0, 2, 4, 6, 11]     
            DWORD   ReceiveProcessState : 3;        //  [17] The state of receive process.
                                                    //  000 = stopped
                                                    //  001 = running   [fetching descriptor]
                                                    //  010 = running   [checking of end of rx packet]
                                                    //  011 = running   [waitinf for rx packet]
                                                    //  100 = suspended [rx buffer unavail]
                                                    //  101 = running   [closing rx descriptor]
                                                    //  110 = running   [flushing current frame from rx FIFO]
                                                    //  111 = running   [queuing rx frame from FIFO --> rx buffer]
            DWORD   TransmissionProcessState : 3;   //  [20] The state of tx process.
                                                    //  000 = stopped
                                                    //  001 = running   [fetching tx descriptor]
                                                    //  010 = running   [waiting for end of tx]
                                                    //  011 = running   [Mem --> FIFO]
                                                    //  100 = reserved
                                                    //  101 = running   [setup packet]
                                                    //  110 = suspended [tx FIFO underflow or tx descriptor unavail]
                                                    //  111 = running   [closing tx descriptor]

                                                                

            DWORD   ErrorBits : 3;                  //  [23] Type of error that caused system error.
                                                    //  000 = Parity Error ... [!!! NEED TO PERFORM S/W RESET !!!]
                                                    //  001 = Master Abort
                                                    //  010 = Target Abort      
    
            DWORD   ________Reserved____ : 6;       
        };
    

        DWORD   dwReg;      
    };

} CSR5_21140;


/////////////////////////////////////////////////////////////////////////////////
//  CSR6 === Operating Mode Register
//

#define     CSR6_MUST_AND   0xe3eeFeFF
#define     CSR6_MUST_OR    0x02000000

typedef struct
{
    union
    {
        struct 
        {   
            DWORD   HashOrPerfectFilteringMode : 1; //  [0 ] Read only... 
            DWORD   StartReceive : 1;               //  [1 ] 1 = rx Process Running  ;  0 = Rx process enters stop state.
            DWORD   HashOnlyFiltering : 1;          //  [2 ] Read only...
            DWORD   PassBadFrames : 1;              //  [3 ] 1 = Pass bad frames mode  
            DWORD   InverseFiltering : 1;           //  [4 ] Read only...
            DWORD   StartOrStopBackoffCounter : 1;  //  [5 ] [suspect read only] 1 indicates internal backoff counter stops 
                                                    //          counting when any carrier activity is detected.
            DWORD   PromiscuousMode : 1;            //  [6 ] 1 = prosmiscuous [default state].
            DWORD   PassAllMulticast : 1;           //  [7 ] 1 = Receive all multicast packets.
    
            DWORD   ________Reserved_ : 1;          //  [8 ] !!! MUST BE ZERO !!!
            DWORD   FullDuplexMode : 1;             //  [9 ] 1 = full duplex mode.
            DWORD   OperatingMode : 2;              //  [10] 00 = Normal  ;  01 = Internal Loop Back   ;   10 = External Loop Back
            DWORD   ForceCollisionMode : 1;         //  [12] Useful only in Internal Loopback Mode.   
            DWORD   StartTransmit : 1;              //  [13] 1 = Tx process running  
    
            DWORD   ThresholdControlBits : 2;       //  [14]
            DWORD   ________Reserved__ : 1;         //  [16] !!! MUST BE ZERO !!!

            DWORD   CaptureEffectEnable : 1;        //  [17] 1 = enable resolution of the capture effect on network.
                                                    //          Work together with CSR6[31]

            DWORD   PortSelect : 1;                 //  [18] 1 = MII/SYM port selected  ;   0 = SRL port selected.
            DWORD   HeartBeatDisable : 1;           //  [19] 1 = SQE disabled <<< MUST BE SET IN MII/SYM mode.
    
            DWORD   ________Reserved___ : 1;        //  [20] !!! MUST BE ZERO !!!
    
            DWORD   StoreAndForward : 1;            //  [21] 1 = Tx starts when full packet risides in FIFO
            DWORD   TransmitThresholdMode : 1;      //  [22] 1 = 10Mbps ;  0 = 100Mbps
            DWORD   PCSfunction : 1;                //  [23] 1 = PCS function active, MII/SYM port operates in symbol mode.
                                                
            DWORD   ScrambleMode : 1;               //  [24] 0 = MII/SYM port is not selected and CSR6[18] is also reset
                                                    //       1 = scramble function active.
    
            DWORD   ________Reserved____ : 1;       //  [25] !!! MUST BE ONE !!! 
            DWORD   ________Reserved_____ : 3;      //  [26] !!! MUST BE ZERO !!!

            DWORD   ________Reserved______ : 1;     //  [29] Reserved.

            DWORD   ReceiveAll : 1;                 //  [30] 1 = receive all regardless of destination address.
    
            DWORD   SpecialCaptureEffectEnable : 1; //  [31] 1 = enables enhanced resolution, work together with CSR6[17]       

        };

        DWORD   dwReg;
    };
    
} CSR6_21140;


/////////////////////////////////////////////////////////////////////////////////
//  CSR7 === Interrupt
//

typedef struct tagCSR7
{
    union
    {
        struct 
        {
    
            DWORD   TxInterruptEnable : 1;          //  [0 ] 1 = enable, work together with CSR7[16] and CSR5[0]
            DWORD   TxStoppedEnable : 1;            //  [1 ] 1 = enable, work together with CSR7[15] and CSR5[1]
            DWORD   TxBufferUnavailableEnable : 1;  //  [2 ] 1 = enable, work together with CSR7[16] and CSR5[2]
            DWORD   TxJabberTimeOutEnable : 1;      //  [3 ] 1 = enable, work together with CSR7[15] and CSR5[3]

            DWORD   ________Reserved_ : 1;
    
            DWORD   UnderFlowInterruptEnable : 1;   //  [5 ] 1 = enable, work together with CSR7[15] and CSR5[5]
            DWORD   ReceiveInterruptEnable : 1;     //  [6 ] 1 = enable, work together with CSR7[16] and CSR5[6]
            DWORD   RxBufferUnavailableEnable : 1;  //  [7 ] 1 = enable, work together with CSR7[15] and CSR5[7]
            DWORD   RxStoppedEnable : 1;            //  [8 ] 1 = enable, work together with CSR7[15] and CSR5[8]
            DWORD   RxWatchdogTimeoutEnable : 1;    //  [9 ] 1 = enable, work together with CSR7[15] and CSR5[9]
            DWORD   EarlyTxInterruptEnable : 1;     //  [10] 1 = enable, work together with CSR7[16] and CSR5[10]
            DWORD   GeneralPurposeTimerEnable : 1;  //  [11] 1 = enable, work together with CSR7[15] and CSR5[11]
    
            DWORD   ________Reserved__ : 1;

            DWORD   FatalBusErrorEnable : 1;        //  [13] 1 = enable, work together with CSR7[15] and CSR5[13]

            DWORD   ________Reserved___ : 1;

            DWORD   AbnormalIntrSummaryEnable : 1;  //  [15] 1 = enable
            DWORD   NormalIntrSummaryEnable : 1;    //  [16] 1 = enable     
        };

        DWORD   dwReg;
    };  

} CSR7_21140;


/////////////////////////////////////////////////////////////////////////////////
//  CSR8 === Missed frames and overflow counter
//

typedef struct tagCSR8
{
    union
    {
        struct
        {   
            DWORD   MissedFrameCounter : 16;        // [0 ] Read only... # frames discarded due to host descriptor not avail                
            DWORD   MissedFrameOverFlow : 1;        // [16] Read only... Sets when missed frame counter overflows.
                                                    //                   Reset when CSR8 is read.
            DWORD   OverflowCounter : 11;           // [17] Read only... # Frames discarded, due to overflow.   
                                                    //                   Counter cleared when read.     
            DWORD   Overflow : 1;                   // [28] Read only... Set when overflow counter overflows.
                                                    //                   Reset when CSR8 is read.
            DWORD   ________Reserved_ : 3;          
        };

        DWORD   dwReg;
    };  

} CSR8_21140;



/////////////////////////////////////////////////////////////////////////////////
//  CSR9 === Boot ROM, Serial ROM, and MII Management Register
//

typedef struct tagCSR9
{
    union
    {
        struct
        {
    
            DWORD   BootRomDataOrSerialRomCtrl : 8; //  [0 ] Data to be read / written to BootROM, if CSR9[12] is set.

            DWORD   ________Reserved_ : 2;

            DWORD   ExternalRegisterSelect : 1;     //  [10] When set 21140 selects an external register.
            DWORD   SerialRomSelect : 1;            //  [11] Select serial rom, work together with CSR9[14] or CSR9[13]
            DWORD   BootRomSelect : 1;              //  [12] 1 = select boot ROM
            DWORD   WriteOperation : 1;             //  [13] 1 = together with CSR9[12], write to boot ROM, serial ROM
                                                    //           and external register.
            DWORD   ReadOperation : 1;              //  [14] 1 = together with CSR9[12], read from boot ROM, serial ROM, 
                                                    //           and external register.

            DWORD   ________Reserved__ : 1;

            DWORD   MiiManagementClock: 1;          //  [16] 1 = mii_mdc is output signal to PHY as timing reference.
            DWORD   MiiManagementWriteData : 1;     //  [17] specifies value of data 21140 writes to PHY.
            DWORD   MiiManagementOperationMode : 1; //  [18] read or write of PHY.
            DWORD   MiiManagementDataIn : 1;        //  [19] to Read data from PHY

            DWORD   ________Reserved___ : 12;               
        };
        
        DWORD   dwReg;
    };
    
} CSR9_21140;


/////////////////////////////////////////////////////////////////////////////////
//  CSR10 === Boot ROM programming address description.
//

typedef struct tagCSR10
{
    union
    {
        struct 
        {   
            DWORD   BootRomAddress : 18 ;           //  [0 ] pointer to boot rom.
            DWORD   ________Reserved_ : 14;     
        };

        DWORD   dwReg;
    };
    
} CSR10_21140;


/////////////////////////////////////////////////////////////////////////////////
//  CSR11 === General Purpose Timer Register
//

typedef struct tagCSR11
{
    union
    {
        struct
        {   
            DWORD   TimerValue : 16;                //  [0 ] Timer value.
            DWORD   ContinuousMode: 1;              //  [16] 1 = Countinuous ; 0 = one shot.
        };

        DWORD   dwReg;
    };
    
} CSR11_21140;


/////////////////////////////////////////////////////////////////////////////////
//  CSR12 === General Purpose Port Register
//

typedef struct tagCSR12
{
    union
    {
        struct
        {
            DWORD   GPIO : 8;                       //  [0 ] 8 bit GPIO
            DWORD   GeneralPurposeControl : 1;      //  [8 ] Mode of GPIO access.
                                                    //      1 == Mode of GPIO (input or output).
                                                    //      0 == Output GPIO data to GPIO pin.
            DWORD   ________Reserved_ : 23;     
        };

        DWORD   dwReg;
    };

} CSR12_21140;


/////////////////////////////////////////////////////////////////////////////////
//  CSR15 === Watchdog Timer Register
//

#define     CSR15_MUST_AND  0xFFFFFeFF


typedef struct tagCSR15
{
    union
    {
        struct
        {
            DWORD   JabberDisable : 1;              //  [0 ] 1 = Transmit jabber function disable.
            DWORD   HostUnjab : 1;                  //  [1 ] 1 = Transmit jabber is released immediately after jabber expiration.
                                                    //       0 = Transmit jabber is released 365 ms to 420 ms after jabber exp.
            DWORD   JabberClock : 1;                //  [2 ] 1 = Tx is cut off after 2048 bytes to 2560 bytes is transmitted.
                                                    //       0 = tx for 10Mbps port is cut off after 26 ms to 33 ms.

            DWORD   ________Reserve_ : 1;

            DWORD   ReceiveWatchdogDisable : 1;     //  [4 ] 1 = disabled
                                                    //       0 = receive carriere longger than 2560 bytes are guaranteed to 
                                                    //           cause wdt counter to time out.
                                                    //           Packets shorter than 2048 bytes are guaranteed to pass.
        
            DWORD   ReceiveWatchdogRelease : 1;     //  [5 ] Time interval no carrier from receive watchdog expiration until
                                                    //       reenabling receive signal.
                                                    //       1 = 40 to 48 bit times from last carrier deassertion.
                                                    //       0 = 16 to 24 bit times from last carrier deassertion.

            DWORD   ________Reserve__ : 2;          

            DWORD   ________MUST_BE_ZERO_ : 1;      //  [8 ] !!! MUST BE ZERO !!!
        };

        DWORD   dwReg;
    };  

} 
CSR15_21140;

/* ------------------------------------------------------------------------------
 *
 *  CSR
 *
 * -------------------------------------------------------------------------------*/


typedef struct tagCSR 
{
    
    CSR0_21140      hwCSR0;     
    PAD(0,4);

    CSR_21140       hwCSR1;         //  Write only... Write with any value and 21140 checks for 
    PAD(1,4);                       //                frames to be transmitted.

    CSR_21140       hwCSR2;         //  Write only... Write with any value and 21140 checks for 
    PAD(2,4);                       //                receive descriptors to be acquired.

    CSR_21140       hwCSR3;         //  Write only... Descriptor List Address Register (Rx) 
    PAD(3,4);

    CSR_21140       hwCSR4;         //  Write only... Descriptor List Address Register (Tx)
    PAD(4,4);
    
    CSR5_21140      hwCSR5; 
    PAD(5,4);

    CSR6_21140      hwCSR6; 
    PAD(6,4);
    
    CSR7_21140      hwCSR7; 
    PAD(7,4);
    
    CSR8_21140      hwCSR8; 
    PAD(8,4);

    CSR9_21140      hwCSR9;     
    PAD(9,4);

    CSR10_21140     hwCSR10;
    PAD(a,4);
    
    CSR11_21140     hwCSR11;        
    PAD(b,4);


    CSR12_21140     hwCSR12;
    PAD(c,4);
#if 0                     //sudhakar
    DWORD           CSR_RESERVED_13;
    PAD(d,4);

    DWORD           CSR_RESERVED_14;
    PAD(e,4);
#endif
    CSR11_21140     hwCSR13;        
    PAD(d,4);


    CSR12_21140     hwCSR14;
    PAD(e,4);


    CSR15_21140     hwCSR15;    
    PAD(f,4);

} CSR, *PCSR;


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//  RX RDES0
//

typedef union _tagRxRdes0__
{
    struct
    {
        DWORD   zero : 1;                       // [0 ]  Should always be zero.
        DWORD   CRCError : 1;                   // [1 ]  CRC error occured in received frame also, when mii_err pin 
                                                //       asserted.   Valid only when RDES0[8] = 1
        DWORD   DribblingBit : 1;               // [2 ]  Frame contained noninteger multiple of 8 bits. Depends on RDES0[8]                                             
        DWORD   ReportOnMiiError : 1;           // [3 ]  rx error in physical layer was reported during frame reception.
        DWORD   ReceiveWatchdog : 1;            // [4 ]  1 = Rx watch dog timer expired while receiving current packet.
                                                //           Valid when RDES0[8] set.
        DWORD   FrameType : 1;                  // [5 ]  1 = Frame is ethernet type frame (frame length is > 1500 bytes).
                                                //       0 = Frame is IEEE 802.3 frame.
                                                //           Valid when RDES0[8] set.
        DWORD   CollisionSeen : 1;              // [6 ]  1 = Frame was damaged by collision (late collision).
                                                //           Valid when RDES0[8] set.
        DWORD   FrameTooLong : 1;               // [7 ]  1 = Frame length > ethernet specified size (1518 bytes).
                                                //           Valid when RDES0[8] set.
        DWORD   LastDescriptor :1;              // [8 ]  1 = Buffers pointed to by this descriptor are last buffers of frame.
        DWORD   FirstDescriptor : 1;            // [9 ]  1 = This descriptor contains the first buffer of a frame.
        DWORD   MulticastFrame : 1;             // [10]  1 = This frame has a multicast address.   Depends on RDES0[8]
        DWORD   RuntFrame : 1;                  // [11]  1 = Frame was damaged by a collision.  
                                                //           This will be set only if CSR6[3] is set.
                                                //           Valid only when RDES0[8] set.
        DWORD   DataType : 2;                   // [12]  00 == Serial Receive Frame.        10 == External loop back.
                                                //       01 == Internal Loop Back Frame.
        DWORD   DescriptorError : 1;            // [14]  1 = Frame truncation caused by a frame does not fit within the 
                                                //           current descriptor buffer, and 21140 does not have access to
                                                //           next descriptor.   Valid only when RDES0[8] set.
        DWORD   ErrorSummary : 1;               // [15]  Logical OR of RDES0[1, 6, 7, 11, 14]
        DWORD   FrameLength : 14;               // [16]  Length (in bytes) of received frame including CRC.
                                                //       Valid only when RDES0[8] is set and RDES[14] is reset.
        DWORD   FilteringFail : 1;              // [30]  Frame failed the address recognition filtering.
        DWORD   OwnBit : 1;                     // [31]  When set 21140 owns the descriptor.   
                                                //       Reset == host owns descriptor.   21140 clears this bit.
    };

    DWORD   dwReg;

} RX_RDES0;


/////////////////////////////////////////////////////////////////////////////////
//  RX RDES1
//

typedef union _tagRxRdes1__
{
    struct 
    {
        DWORD   Buffer1Size : 11;               // [0 ] Buffer size MUST BE multiple of 4
        DWORD   Buffer2Size : 11;               // [11] Buffer size MUST BE multiple of 4, not valid if RDES1[24] set.

        DWORD   ________Reserve_ : 2;           

        DWORD   SecondAddressChained : 1;       // [24] 1 == Second address in the descriptor is the next descriptor addr.
                                                //      0 == Second address is second buffer of this descriptor.

        DWORD   ReceiveEndOfRing : 1;           // [25] 1 == Descriptor List reached its final descriptor.
    };

    DWORD   dwReg;

} RX_RDES1;


/* ------------------------------------------------------------------------------
 *
 *  RX RDES
 *
 * -------------------------------------------------------------------------------*/


typedef struct  _tagRxDescriptor__
{
    RX_RDES0    RDES0;      
    RX_RDES1    RDES1;
    DWORD       RDES2;      // Buffer address 1
    DWORD       RDES3;      // Buffer address 2

} RX_DESCRIPTOR_FORMAT, *PRX_DESCRIPTOR_FORMAT;



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//  TX TDES0
//

typedef union _tagTxTdes0__
{
    struct
    {
        DWORD   Deferred : 1;                   // [0 ] 1 == 21140 had to defer because carrier was asserted.
        DWORD   UnderFlow : 1;                  // [1 ] 1 == Transmitter aborted message because data arrived late from
                                                //           memory.
        DWORD   LinkFailReport : 1;             // [2 ] 1 == Link test failed before frame was transmitted through symbol
                                                //           port.   Valid only while using symbol mode CSR6[23] == 1
        DWORD   CollisionCount : 4;             // [3 ] # of collision b4 frame was transmitted.
        DWORD   HeartbeatFail : 1;              // [7 ] Effective only in 10Mbps.
        DWORD   ExcessiveCollision : 1;         // [8 ] Transmission was aborted after 16 successive collisions.
        DWORD   LateCollision : 1;              // [9 ] 1 == Late collision happened.
        DWORD   NoCarrier : 1;                  // [10] 1 == Carrier signal from transceiver was not preset during 
                                                //           transmission.
        DWORD   LossOfCarrier : 1;              // [11] 1 == Loss of carrier during transmission.

        DWORD   ________Reserved_ : 2;

        DWORD   TransmitJabberTimeout : 1;      // [14] 1 === Transmit jabber timer timed out.
        DWORD   ErrorSummary : 1;               // [15] Logical OR of TDES0[1, 8, 9, 10, 11, 14]

        DWORD   ________Reserved__ : 15 ;       
        
        DWORD   OwnBit : 1;                     // [31] 1 == Descriptor owned by 21140.

    };

    DWORD   dwReg;

} TX_TDES0;


/////////////////////////////////////////////////////////////////////////////////
//  TX TDES1
//

typedef union _tagTxTdes1__
{
    struct
    {
        DWORD   Buffer1Size : 11;               //  [0 ]    
        DWORD   Buffer2Size : 11;               //  [11]

        DWORD   FilteringType0: 1;              //  [22] FT0 and FT1 (TEDES1[28]) works together.

        DWORD   DisablePadding : 1;             //  [23] 1 == 21140 does not automatically add a padding field 
                                                //            to < 64 bytes packet.
        DWORD   SecondAddressChained : 1;       //  [24] 1 == Second address is next descriptor rather than second buf addr
        DWORD   TransmitEndOfRing : 1;          //  [25]
        DWORD   AddCRCDisable : 1;              //  [26]
        DWORD   SetupPacket : 1;                //  [27] 1 == Indicates that the current descriptor is a setup frame descriptor.
        
        DWORD   FilteringType1: 1;              //  [28] Works together with TDES1[22]
        DWORD   FirstSegment : 1;               //  [29] 1 == Buffer contains first segment of a frame.
        DWORD   LastSegment : 1;                //  [30] 1 == Buffer contains last segment of a frame.
        DWORD   InterruptOnCompletion : 1;      //  [31] 1 == 21140 will set interrupt after this frame has been xmitted.
                                                //            Valid only when TDES1[30] is set or it is a setup packet.

    };
    
    DWORD   dwReg;
} TX_TDES1;



/* ------------------------------------------------------------------------------
 *
 *  RX RDES
 *
 * -------------------------------------------------------------------------------*/


typedef struct  _tagTxDescriptor__
{
    TX_TDES0    TDES0;      
    TX_TDES1    TDES1;  
    DWORD       TDES2;      // Buffer address 1
    DWORD       TDES3;      // Buffer address 2
} TX_DESCRIPTOR_FORMAT, *PTX_DESCRIPTOR_FORMAT;


/////////////////////////////////////////////////////////////////////////////////
//  Defines for descriptors.
//


#define MAX_BUFFER_SIZE                     1536                        // 12*128(=max cache line size)
#define DESC_OWNED_BY_HOST                  ((ULONG)(0x7fffffff))
#define DESC_OWNED_BY_DEC21140              ((ULONG)(0x80000000))
#define SECOND_ADDRESS_CHAINED              (ULONG) (1 << 24)




/////////////////////////////////////////////////////////////////////////////////
//
//  Error returned from exported function...
//

#define DEC21140_ERROR_TX_LOOPBACK      0x00000001  




/////////////////////////////////////////////////////////////////////////////////
//  Exported funtions to caller
//
//
BOOL    DEC21140Init (UINT8 *pbBaseAddress, UINT32 dwLogicalLocation, UINT16 MacAddr[3]);
void    DEC21140EnableInts ();
void    DEC21140DisableInts ();
UINT16  DEC21140GetFrame (BYTE *pbData, UINT16 *pwLength);
UINT16  DEC21140SendFrame (BYTE *pbData, DWORD dwLength);
                          
#pragma warning(pop)

#endif 
// _DEC21140HARDWARE_
