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
/**************************************************************************

Copyright 1999-2002 Intel Corporation

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

    Xllp_PCCardSocket.c

Abstract:

    This file implements the common cross-platform API for
    the PC Card interface on the BULVERDE-MAINSTONE platform.
        
Functions defined in this file:
    XllpPCCardHWSetup()
    XllpPCCardConfigureGPIOs()
    XllpPCCardGetSocketState()
    XllpPCCardResetSocket()
    XllpPCCardPowerOn()
    XllpPCCardPowerOff()
    XllpPCCardGetVoltageSetting()


    
**************************************************************************/


#include "Xllp_PCCardSocket.h"


/******************************************************************************

  Function Name: XllpPCCardHWSetup

  Description: Performs the required hardware setup for the PC Card interface
               to configure all sockets properly for use. At present, both
               sockets are configured on the MAINSTONE platform. In particular,
               the following hardware setup is performed:

               1. Configures all BULVERDE GPIOs appropriately for the PC Card
                  interface.
               2. Defaults initial power supply to each socket to 3.3V.
               3. Routes mainboard signals to the PC Card interface (as opposed
                  to the Baseband controller).


  Global Registers Modified:
    NONE

  Input Arguments:
    XLLP_PCCARDSOCKET_T *pstrSocketHandle: Pointer to the global PC Card handle

  Output Arguments:
    NONE

  Return Value:


*******************************************************************************/
XLLP_STATUS_T XllpPCCardHWSetup(XLLP_PCCARDSOCKET_T *pstrSocketHandle)
{
    volatile XLLP_BCR_T *vpstrMainBLRegs = (volatile XLLP_BCR_T *)pstrSocketHandle->pstrBcrHandle;
    XLLP_STATUS_T ReturnValue = XLLP_STATUS_SUCCESS;
    XLLP_UINT32_T uiCardVoltage = 0;

    //
    //Check the validity of the input arguments to the function
    //
    if((pstrSocketHandle == XLLP_NULL_PTR))
    {
        ReturnValue = XLLP_STATUS_PCCARD_FAILURE;   
    }
    
    if(ReturnValue != XLLP_STATUS_PCCARD_FAILURE)
    {
        XllpPCCardConfigureGPIOs(pstrSocketHandle); 

        //
        //*************************** SOCKET 0 **********************************
        //
        vpstrMainBLRegs->PCMCIAS0SCR &= ~(XLLP_BCR_PCMCIA_SCR_S0_PWR);

        //
        //Deassert RESET for Socket 0: bit[4] of the Socket 0 Status Control Register
        //
        vpstrMainBLRegs->PCMCIAS0SCR &= ~(XLLP_BCR_PCMCIA_SCR_S0_RESET);

        //
        //If a card is inserted, configure the MAX1602EE power IC for
        //appropriate power supply to the socket, based on the voltage requirements
        //of the card. If no card is present in the socket, then bypass powering
        //up the socket at this point
        //
        ReturnValue = XllpPCCardGetVoltageSetting(pstrSocketHandle, XLLP_PCCARD_SOCKET0, &uiCardVoltage);

        if(ReturnValue == XLLP_STATUS_SUCCESS)
        {
            XllpPCCardPowerOn(pstrSocketHandle, XLLP_PCCARD_SOCKET0, uiCardVoltage);

            //
            //Assert RESET for Socket 0: bit[4] of the Socket 0 Status Control Register
            //
            vpstrMainBLRegs->PCMCIAS0SCR |= XLLP_BCR_PCMCIA_SCR_S0_RESET;

            //
            //Deassert RESET for Socket 0: bit[4] of the Socket 0 Status Control Register
            //
            vpstrMainBLRegs->PCMCIAS0SCR &= ~(XLLP_BCR_PCMCIA_SCR_S0_RESET);
        }

        //
        //*************************** SOCKET 1 **********************************
        //
        vpstrMainBLRegs->PCMCIAS1SCR &= ~(XLLP_BCR_PCMCIA_SCR_S1_PWR);

        //
        //Deassert RESET for Socket 1: bit[4] of the Socket 1 Status Control Register
        //
        vpstrMainBLRegs->PCMCIAS1SCR &= ~(XLLP_BCR_PCMCIA_SCR_S1_RESET);

        //
        //If a card is inserted, configure the MAX1602EE power IC for
        //appropriate power supply to the socket, based on the voltage requirements
        //of the card. If no card is present in the socket, then bypass powering
        //up the socket at this point
        //
        uiCardVoltage = 0;   //reset the value
        ReturnValue = XLLP_STATUS_SUCCESS;   //reset the value

        ReturnValue = XllpPCCardGetVoltageSetting(pstrSocketHandle, XLLP_PCCARD_SOCKET1, &uiCardVoltage);

        if(ReturnValue == XLLP_STATUS_SUCCESS)
        {
            XllpPCCardPowerOn(pstrSocketHandle, XLLP_PCCARD_SOCKET1, uiCardVoltage);

            //
            //Assert RESET for Socket 1: bit[4] of the Socket 1 Status Control Register
            //
            vpstrMainBLRegs->PCMCIAS1SCR |= XLLP_BCR_PCMCIA_SCR_S0_RESET;

            //
            //Deassert RESET for Socket 1: bit[4] of the Socket 1 Status Control Register
            //
            vpstrMainBLRegs->PCMCIAS1SCR &= ~(XLLP_BCR_PCMCIA_SCR_S0_RESET);
        }

        //
        //Route signals to the PC card interface, as opposed to the Baseband controller
        //
        vpstrMainBLRegs->MISCWR1 |= (XLLP_BCR_MISCWR1_BB_SEL);
        vpstrMainBLRegs->MISCWR1 &= ~(XLLP_BCR_MISCWR1_BB_SEL);
    }

    return ReturnValue;

} //end XllpPCCardHWSetup()



/******************************************************************************

  Function Name: XllpPCCardConfigureGPIOs

  Description: Configures BULVERDE's GPIOs for the PC Card interface

  Global Registers Modified:

  Input Arguments:

  Output Arguments:

  Return Value:


*******************************************************************************/
void XllpPCCardConfigureGPIOs(XLLP_PCCARDSOCKET_T *pstrSocketHandle)
{
    XLLP_UINT32_T ulLockID;
    volatile XLLP_GPIO_T *vpstrBvdGPIORegs = (volatile XLLP_GPIO_T *)pstrSocketHandle->pstrGpioRegsHandle;


    //
    //Configure GPIO Output Set registers for active-low, output GPIO pins.This is a required
    //step for programming Bulverde GPIOs.
    //
    ulLockID = XllpLock(GPSR1);
    vpstrBvdGPIORegs->GPSR1 |= (XLLP_GPIO_BIT_PCMCIA_nPOE |
                                XLLP_GPIO_BIT_nPWE |
                                XLLP_GPIO_BIT_PCMCIA_nPIOR |
                                XLLP_GPIO_BIT_PCMCIA_nPIOW |
                                XLLP_GPIO_BIT_PCMCIA_nPCE2 |
                                XLLP_GPIO_BIT_PCMCIA_nPREG);
    XllpUnlock(ulLockID);

    ulLockID = XllpLock(GPSR2);
    vpstrBvdGPIORegs->GPSR2 |= (XLLP_GPIO_BIT_PCMCIA_PSKTSEL |
                                XLLP_GPIO_BIT_PCMCIA_nPCE1);
    XllpUnlock(ulLockID);

    //
    //Configure GPIO pin directions
    //
    //PC Card interface GPIO Output pins: GPIO 48, 49, 50, 51, 55, 78 and 79
    //PC Card interface GPIO Input pins:  GPIO 56 and 57
    ulLockID = XllpLock(GPDR1);
    vpstrBvdGPIORegs->GPDR1 |= (XLLP_GPIO_BIT_PCMCIA_nPOE |
                                XLLP_GPIO_BIT_nPWE |
                                XLLP_GPIO_BIT_PCMCIA_nPIOR |
                                XLLP_GPIO_BIT_PCMCIA_nPIOW |
                                XLLP_GPIO_BIT_PCMCIA_nPCE2 |
                                XLLP_GPIO_BIT_PCMCIA_nPREG);

    vpstrBvdGPIORegs->GPDR1 &= ~(XLLP_GPIO_BIT_PCMCIA_nPWAIT);
    vpstrBvdGPIORegs->GPDR1 &= ~(XLLP_GPIO_BIT_PCMCIA_nIOIS16);
    XllpUnlock(ulLockID);

    ulLockID = XllpLock(GPDR2);
    vpstrBvdGPIORegs->GPDR2 |= XLLP_GPIO_BIT_PCMCIA_PSKTSEL;
    vpstrBvdGPIORegs->GPDR2 |= XLLP_GPIO_BIT_PCMCIA_nPCE1;
    XllpUnlock(ulLockID);

    //
    //Configure GPIO pin alternate functions
    //
    ulLockID = XllpLock(GAFR1_U);
    vpstrBvdGPIORegs->GAFR1_U &= 0xFFF00F00;  //Clear the alternate function bits
                                              //for GPIO 48, 49, 50, 51, 55, 56 and 57
    //Set Alternate Function 2 for GPIO 48, 49, 50, 51 and 55
    vpstrBvdGPIORegs->GAFR1_U |= (XLLP_GPIO_AF_BIT_PCMCIA_nPOE |
                                  XLLP_GPIO_AF_BIT_nPWE |
                                  XLLP_GPIO_AF_BIT_PCMCIA_nPIOR |
                                  XLLP_GPIO_AF_BIT_PCMCIA_nPIOW |
                                  XLLP_GPIO_AF_BIT_PCMCIA_nPCE2 |
                                  XLLP_GPIO_AF_BIT_PCMCIA_nPREG);

    //Set Alternate Function 1 for GPIO 56 and 57
    vpstrBvdGPIORegs->GAFR1_U |= (XLLP_GPIO_AF_BIT_PCMCIA_nPWAIT |
                                  XLLP_GPIO_AF_BIT_PCMCIA_nIOIS16);
    XllpUnlock(ulLockID);

    ulLockID = XllpLock(GAFR2_L);
    vpstrBvdGPIORegs->GAFR2_L &= 0x3FFFFFFF;  //Clear the alternate function bits
                                              //for GPIO 79
    //Set Alternate Function 1 for GPIO 79
    vpstrBvdGPIORegs->GAFR2_L |= XLLP_GPIO_AF_BIT_PCMCIA_PSKTSEL;
    XllpUnlock(ulLockID);

    ulLockID = XllpLock(GAFR2_U);
    vpstrBvdGPIORegs->GAFR2_U &= 0xFFFFF3FF;  //Clear the alternate function bits for GPIO 85
    vpstrBvdGPIORegs->GAFR2_U |= XLLP_GPIO_AF_BIT_PCMCIA_nPCE1;
    XllpUnlock(ulLockID);

} //end XllpPCCardConfigureGPIOs()



/******************************************************************************

  Function Name: XllpPCCardGetSocketState

  Description:

  Global Registers Modified:

  Input Arguments:

  Output Arguments:

  Return Value:


*******************************************************************************/
XLLP_STATUS_T XllpPCCardGetSocketState(XLLP_PCCARDSOCKET_T *pstrSocketHandle,
                                       XLLP_VUINT16_T       ushSocketNumber)
{

    XLLP_STATUS_T ReturnValue = XLLP_STATUS_SUCCESS;


    //Check the validity of the input arguments to the function
    if((ushSocketNumber > XLLP_MAINSTONE_MAX_PCCARD_SOCKETS) ||
       (pstrSocketHandle == XLLP_NULL_PTR))
    {
        ReturnValue = XLLP_STATUS_PCCARD_FAILURE;   
    }

    if(ReturnValue != XLLP_STATUS_PCCARD_FAILURE)
    {
        switch(ushSocketNumber)
        {
            case XLLP_PCCARD_SOCKET0:

                //
                //Check the CD status bit[5] of the Socket 0 Status Register. If set, it indicates
                //that a card is either not present or is not properly inserted in the socket. If clear,
                //it indicates that a card is present in the socket.
                //
                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) & (XLLP_BCR_PCMCIA_SCR_S0_nCD))
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket0CDState = XLLP_FALSE;
                }
                else
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket0CDState = XLLP_TRUE;
                }

                //
                //Check the BVD1 status bit[8] of the Socket 0 Status Register. If set, then it
                //indicates that the card status has not changed. If clear, then it indicates that
                //a card status change event has occurred.
                //
                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) & (XLLP_BCR_PCMCIA_SCR_S0_nSTSCHG_BVD1))
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket0BVD1State = XLLP_FALSE;
                }
                else
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket0BVD1State = XLLP_TRUE;
                }

                //
                //Check the BVD2 status bit[9] of the Socket 0 Status Register. If set, then it
                //indicates that the card status has not changed. If clear, then it indicates that
                //a card status change event has occurred.
                //
                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) & (XLLP_BCR_PCMCIA_SCR_S0_nSPKR_BVD2))
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket0BVD2State = XLLP_FALSE;
                }
                else
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket0BVD2State = XLLP_TRUE;
                }

                //
                //Check the IREQ status bit[10] of the Socket 0 Status Register. If set, then it
                //indicates that the card is READY (i.e. no interrupt request is pending). If clear,
                //it indicates that the card is BUSY, pending an interrupt request.
                //
                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) & (XLLP_BCR_PCMCIA_SCR_S0_nIRQ))
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket0IREQState = XLLP_TRUE;
                }
                else
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket0IREQState = XLLP_FALSE;
                }

                break;

            case XLLP_PCCARD_SOCKET1:

                //
                //Check the CD status bit[5] of the Socket 1 Status Register. If set, it indicates
                //that a card is either not present or is not properly inserted in the socket. If clear,
                //it indicates that a card is present in the socket.
                //
                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) & (XLLP_BCR_PCMCIA_SCR_S1_nCD))
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket1CDState = XLLP_FALSE;
                }
                else
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket1CDState = XLLP_TRUE;
                }

                //
                //Check the BVD1 status bit[8] of the Socket 1 Status Register. If set, then it
                //indicates that the card status has not changed. If clear, then it indicates that
                //a card status change event has occurred.
                //
                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) & (XLLP_BCR_PCMCIA_SCR_S1_nSTSCHG_BVD1))
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket1BVD1State = XLLP_FALSE;
                }
                else
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket1BVD1State = XLLP_TRUE;
                }

                //
                //Check the BVD2 status bit[9] of the Socket 1 Status Register. If set, then it
                //indicates that the card status has not changed. If clear, then it indicates that
                //a card status change event has occurred.
                //
                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) & (XLLP_BCR_PCMCIA_SCR_S1_nSPKR_BVD2))
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket1BVD2State = XLLP_FALSE;
                }
                else
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket1BVD2State = XLLP_TRUE;
                }

                //
                //Check the IREQ status bit[10] of the Socket 1 Status Register. If set, then it
                //indicates that the card is READY (i.e. no interrupt request is pending). If clear,
                //it indicates that the card is BUSY, pending an interrupt request.
                //
                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) & (XLLP_BCR_PCMCIA_SCR_S1_nIRQ))
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket1IREQState = XLLP_TRUE;
                }
                else
                {
                    pstrSocketHandle->pstrPCCardSocketState->blSocket1IREQState = XLLP_FALSE;
                }

                break;

            default:
                break;

        } //end switch(uSocket)

    } //end if

    return ReturnValue;

} //end XllpPCCardGetSocketState()


/******************************************************************************

  Function Name: XllpPCCardResetSocket

  Description:

  Global Registers Modified:

  Input Arguments:

  Output Arguments:

  Return Value:


*******************************************************************************/
XLLP_STATUS_T XllpPCCardResetSocket(XLLP_PCCARDSOCKET_T *pstrSocketHandle,
                                    XLLP_VUINT16_T       ushSocketNumber)
{
    XLLP_STATUS_T ReturnValue = XLLP_STATUS_SUCCESS;
    XLLP_BOOL_T blCDStatus = XLLP_FALSE;
    XLLP_BOOL_T blRDYStatus = XLLP_FALSE;
    XLLP_UINT16_T t; //loop counter

    //Check the validity of the input arguments to the function
    if((ushSocketNumber > XLLP_MAINSTONE_MAX_PCCARD_SOCKETS) ||
       (pstrSocketHandle == XLLP_NULL_PTR))
    {
        ReturnValue = XLLP_STATUS_PCCARD_FAILURE;   
    }

    if(ReturnValue != XLLP_STATUS_PCCARD_FAILURE)
    {
        switch(ushSocketNumber)
        {
            case XLLP_PCCARD_SOCKET0:

                //
                //Check if a card is inserted in the socket
                //
                blCDStatus = ((pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) & (XLLP_BCR_PCMCIA_SCR_S0_nCD));

                //
                //If the CD status bit is *not* set in the PC Card Status register,
                //it implies that a card is properly inserted. Reset the socket
                //in that case.
                //
                if(!blCDStatus)
                {
                    //Assert reset
                    (pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) |= (XLLP_BCR_PCMCIA_SCR_S0_RESET);

					// Wait long enough for the device to notice.
                    XllpOstDelayMilliSeconds(pstrSocketHandle->pstrOstRegsHandle,200);

                    //Clear reset
                    (pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) &= ~(XLLP_BCR_PCMCIA_SCR_S0_RESET);

                    //Wait for the READY signal to be set.This will indicate that the socket
                    XllpOstDelayMilliSeconds(pstrSocketHandle->pstrOstRegsHandle,20);
                    //is interrupt-ready and can function normally now.
                    for(t = 0; t < XLLP_PCCARD_MAX_READY_WAIT_TIME;  t += XLLP_PCCARD_READY_POLL_INTERVAL)
                    {
                        blRDYStatus = ((pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) &
                                                             (XLLP_BCR_PCMCIA_SCR_S0_nIRQ));

                        if(!blRDYStatus)
                        {
                            XllpOstDelayMilliSeconds(pstrSocketHandle->pstrOstRegsHandle,
                                                     XLLP_PCCARD_READY_POLL_INTERVAL);                          
                        }
                        else
                        {
                            break;
                        }
                    } //end for loop

                }
                //If no card is inserted or if a card is not properly inserted, then tri-
                //state the socket.
                else
                {
                    //Tri-state the socket here.
                    ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
                }

            break;

            case XLLP_PCCARD_SOCKET1:
                //Check if a card is inserted in the socket
                blCDStatus = ((pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) &
                                                     (XLLP_BCR_PCMCIA_SCR_S1_nCD));

                //If the CD status bit is *not* set in the PC Card Status register,
                //it implies that a card is properly inserted. Reset the socket
                //in that case.
                if(!blCDStatus)
                {
                    //Assert reset
                    (pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) |=
                                          (XLLP_BCR_PCMCIA_SCR_S1_RESET);

					// Wait long enough for the device to notice.
                    XllpOstDelayMilliSeconds(pstrSocketHandle->pstrOstRegsHandle,200);

                    //Clear reset
                    (pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) &=
                                         ~(XLLP_BCR_PCMCIA_SCR_S1_RESET);
                    XllpOstDelayMilliSeconds(pstrSocketHandle->pstrOstRegsHandle,20);


                    //Wait for the READY signal to be set.This will indicate that the socket
                    //is interrupt-ready and can function normally now.
                    for(t = 0; t < XLLP_PCCARD_MAX_READY_WAIT_TIME;  t += XLLP_PCCARD_READY_POLL_INTERVAL)
                    {
                        blRDYStatus = ((pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) &
                                                             (XLLP_BCR_PCMCIA_SCR_S1_nIRQ));

                        if(!blRDYStatus)
                        {
                            XllpOstDelayMilliSeconds(pstrSocketHandle->pstrOstRegsHandle,
                                                     XLLP_PCCARD_READY_POLL_INTERVAL);
                        }
                        else
                        {
                            break;
                        }
                    } //end for loop

                }
                //If no card is inserted or if a card is not properly inserted, then tri-
                //state the socket.
                else
                {
                    //Tri-state the socket here
                    ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
                }

            break;

            default:
                //TBD. Should not be here at all!!
                ReturnValue = XLLP_STATUS_PCCARD_FAILURE;

            break;
        } //end switch

    } //end if

    return ReturnValue;

} //end XllpPCCardResetSocket()



/******************************************************************************

  Function Name: XllpPCCardPowerOn

  Description:

  Global Registers Modified:

  Input Arguments:

  Output Arguments:

  Return Value:


*******************************************************************************/
XLLP_STATUS_T XllpPCCardPowerOn(XLLP_PCCARDSOCKET_T *pstrSocketHandle,
                                XLLP_VUINT16_T      ushSocketNumber,
                                XLLP_UINT32_T       uiCardVoltage)
{
    XLLP_STATUS_T ReturnValue = XLLP_STATUS_SUCCESS;



    //
    //Check the validity of the input arguments to the function
    //
    if((ushSocketNumber > XLLP_MAINSTONE_MAX_PCCARD_SOCKETS) ||
       (pstrSocketHandle == XLLP_NULL_PTR))
    {
        ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
    }

    if(ReturnValue != XLLP_STATUS_PCCARD_FAILURE)
    {
        switch(ushSocketNumber)
        {
            case XLLP_PCCARD_SOCKET0:

                if(uiCardVoltage == XLLP_PCCARD_5_00VOLTS)
                {
                    //
                    //5V card detected; set bit[2] of the status register
                    //
                    pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR |= (XLLP_BIT_2);
                    pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR &= ~(XLLP_BIT_3);
                }
                else if(uiCardVoltage == XLLP_PCCARD_3_30VOLTS)
                {
                    //
                    //3.3V card detected; set bit[3] of the status register
                    //
                    pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR |= (XLLP_BIT_3);
                    pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR &= ~(XLLP_BIT_2);
                }
                else
                {
                    //
                    //Unsupported PC Card voltage
                    //
                    ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
                }
                (pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) &= ~(XLLP_BCR_PCMCIA_SCR_S0_RESET);

                break;

            case XLLP_PCCARD_SOCKET1:

                if(uiCardVoltage == XLLP_PCCARD_5_00VOLTS)
                {
                    //5V card detected; set bit[2] of the status register
                    pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR |= (XLLP_BIT_2);
                    pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR &= ~(XLLP_BIT_3);
                }
                else if(uiCardVoltage == XLLP_PCCARD_3_30VOLTS)
                {
                    //3.3V card detected; set bit[3] of the status register
                    pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR |= (XLLP_BIT_3);
                    pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR &= ~(XLLP_BIT_2);
                }
                else
                {
                    //
                    //Unsupported PC Card voltage
                    //
                    ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
                }
                (pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) &= ~(XLLP_BCR_PCMCIA_SCR_S1_RESET);
                break;

            default:
                //TBD. Should not be here at all!!
                ReturnValue = XLLP_STATUS_PCCARD_FAILURE;

            break;
        } //end switch
    }

    return ReturnValue;

} //end XllpPCCardEnableSocket()



/******************************************************************************

  Function Name: XllpPCCardPowerOff

  Description:

  Global Registers Modified:

  Input Arguments:

  Output Arguments:

  Return Value:


*******************************************************************************/
XLLP_STATUS_T XllpPCCardPowerOff(XLLP_PCCARDSOCKET_T *pstrSocketHandle,
                                 XLLP_VUINT16_T      ushSocketNumber)
{
    XLLP_STATUS_T ReturnValue = XLLP_STATUS_SUCCESS;



    //
    //Check the validity of the input arguments to the function
    //
    if((ushSocketNumber > XLLP_MAINSTONE_MAX_PCCARD_SOCKETS) ||
       (pstrSocketHandle == XLLP_NULL_PTR))
    {
        ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
    }

    if(ReturnValue != XLLP_STATUS_PCCARD_FAILURE)
    {
        switch(ushSocketNumber)
        {
            case XLLP_PCCARD_SOCKET0:

                //
                //Power off the socket
                //
                pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR |= (XLLP_BIT_2);
                pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR |= (XLLP_BIT_3);

                break;

            case XLLP_PCCARD_SOCKET1:

                //
                //Power off the socket
                //
                pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR |= (XLLP_BIT_2);
                pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR |= (XLLP_BIT_3);

                break;

            default:

                //TBD. Should not be here at all!!
                ReturnValue = XLLP_STATUS_PCCARD_FAILURE;

            break;

        } //end switch
    } //end if

    return ReturnValue;

} //end XllpPCCardResetSocket()



/******************************************************************************

  Function Name: XllpPCCardGetVoltageSetting

  Description:

  Global Registers Modified:

  Input Arguments:

  Output Arguments:

  Return Value:


*******************************************************************************/
XLLP_STATUS_T XllpPCCardGetVoltageSetting(XLLP_PCCARDSOCKET_T *pstrSocketHandle,
                                          XLLP_VUINT16_T ushSocketNumber,
                                          XLLP_UINT32_T *puiCardVoltage)
{
    XLLP_STATUS_T ReturnValue = XLLP_STATUS_SUCCESS;



    //
    //Check the validity of the input arguments to the function
    //
    if((ushSocketNumber > XLLP_MAINSTONE_MAX_PCCARD_SOCKETS) ||
       (pstrSocketHandle == XLLP_NULL_PTR))
    {
        ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
    }

    if(ReturnValue != XLLP_STATUS_PCCARD_FAILURE)
    {
        switch(ushSocketNumber)
        {
            case XLLP_PCCARD_SOCKET0:

                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) & (XLLP_BCR_PCMCIA_SCR_S0_nCD))
                {
                    ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
                }
                else
                {
                    //
                    //Determine the voltage requirements of the PC Card, else
                    //default voltage to 3.3V
                    //
                    if(((pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) & (XLLP_BIT_6)) &&
                       ((pstrSocketHandle->pstrBcrHandle->PCMCIAS0SCR) & (XLLP_BIT_7)))
                    {
                        //
                        //5 volt card detected in the socket
                        //
                        *puiCardVoltage = XLLP_PCCARD_5_00VOLTS;
                    }
                    else
                    {
                        //
                        //3.3 volt card detected in the socket
                        //
                        *puiCardVoltage = XLLP_PCCARD_3_30VOLTS;
                    }
                }

                break;

            case XLLP_PCCARD_SOCKET1:

                if((pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) & (XLLP_BCR_PCMCIA_SCR_S0_nCD))
                {
                    ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
                }
                else
                {
                    //
                    //Determine the voltage requirements of the PC Card, else
                    //default voltage to 3.3V
                    //
                    if(((pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) & (XLLP_BIT_6)) &&
                       ((pstrSocketHandle->pstrBcrHandle->PCMCIAS1SCR) & (XLLP_BIT_7)))
                    {
                        //
                        //5 volt card detected in the socket
                        //
                        *puiCardVoltage = XLLP_PCCARD_5_00VOLTS;
                    }
                    else
                    {
                        //
                        //3.3 volt card detected in the socket
                        //
                        *puiCardVoltage = XLLP_PCCARD_3_30VOLTS;
                    }
                }

                break;

            default:

                //TBD. Should not be here at all!!
                ReturnValue = XLLP_STATUS_PCCARD_FAILURE;

            break;

        } //end switch
    }

    return ReturnValue;

} //end XllpPCCardGetVoltageSetting()



/******************************************************************************

  Function Name: XllpPCCardSetExpMemTiming

  Description:

  Global Registers Modified:

  Input Arguments:

  Output Arguments:

  Return Value:


*******************************************************************************/
XLLP_STATUS_T XllpPCCardSetExpMemTiming(XLLP_PCCARDSOCKET_T *pstrSocketHandle)
{

    XLLP_STATUS_T ReturnValue = XLLP_STATUS_SUCCESS;

    //
    //Check the validity of the input arguments to the function
    //
    if(pstrSocketHandle == XLLP_NULL_PTR)
    {
        ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
    }

    if(ReturnValue != XLLP_STATUS_PCCARD_FAILURE)
    {
        pstrSocketHandle->pstrMemCtrlRegsHandle->MCATT0 = pstrSocketHandle->pstrCardTimingHandle->MCATT0;
        pstrSocketHandle->pstrMemCtrlRegsHandle->MCATT1 = pstrSocketHandle->pstrCardTimingHandle->MCATT1;

        pstrSocketHandle->pstrMemCtrlRegsHandle->MCMEM0 = pstrSocketHandle->pstrCardTimingHandle->MCMEM0;
        pstrSocketHandle->pstrMemCtrlRegsHandle->MCMEM1 = pstrSocketHandle->pstrCardTimingHandle->MCMEM1;

        pstrSocketHandle->pstrMemCtrlRegsHandle->MCIO0 = pstrSocketHandle->pstrCardTimingHandle->MCIO0;
        pstrSocketHandle->pstrMemCtrlRegsHandle->MCIO1 = pstrSocketHandle->pstrCardTimingHandle->MCIO1;
    }

    return ReturnValue;

} //end XllpPCCardSetExpMemTiming()



/******************************************************************************

  Function Name: XllpPCCardGetExpMemTiming

  Description:

  Global Registers Modified:

  Input Arguments:

  Output Arguments:

  Return Value:


*******************************************************************************/
XLLP_STATUS_T XllpPCCardGetExpMemTiming(XLLP_PCCARDSOCKET_T *pstrSocketHandle,
                                        XLLP_CARDTIMING_T *pstrCardTimingHandle)
{
    XLLP_STATUS_T ReturnValue = XLLP_STATUS_SUCCESS;

    //
    //Check the validity of the input arguments to the function
    //
    if((pstrSocketHandle == XLLP_NULL_PTR) ||
       (pstrCardTimingHandle == XLLP_NULL_PTR))
    {
        ReturnValue = XLLP_STATUS_PCCARD_FAILURE;
    }

    if(ReturnValue != XLLP_STATUS_PCCARD_FAILURE)
    {
        pstrCardTimingHandle->MCATT0 = pstrSocketHandle->pstrMemCtrlRegsHandle->MCATT0;
        pstrCardTimingHandle->MCATT1 = pstrSocketHandle->pstrMemCtrlRegsHandle->MCATT1;

        pstrCardTimingHandle->MCMEM0 = pstrSocketHandle->pstrMemCtrlRegsHandle->MCMEM0;
        pstrCardTimingHandle->MCMEM1 = pstrSocketHandle->pstrMemCtrlRegsHandle->MCMEM1;

        pstrCardTimingHandle->MCIO0 = pstrSocketHandle->pstrMemCtrlRegsHandle->MCIO0;
        pstrCardTimingHandle->MCIO1 = pstrSocketHandle->pstrMemCtrlRegsHandle->MCIO1;
    }

    return ReturnValue;

} //end XllpPCCardGetExpMemTiming()






