//------------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------
//
//  File:  soc_ddk_gpio.c
//
//  This file contains the SoC-specific DDK interface for the GPIO module.
//
//-----------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4115 4201 4204 4214)
#include <windows.h>
#include <ceddk.h>
#pragma warning(pop)
#include "csp.h"

//-----------------------------------------------------------------------------
// External Functions


//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines
#define DDK_GPIO_GET_BANK(x) (x >> 5)
#define DDK_GPIO_GET_PIN(x)  (x & 0x1F)
#define DDK_GPIO_GET_BIT(x)  (1 << (x & 0x1F))

//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables
// Following variables define the GPIO SoC-specific features.
extern PVOID pv_HWregPINCTRL;

//-----------------------------------------------------------------------------
// Local Variables

//-----------------------------------------------------------------------------
// Local Functions


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioConfig
//
//  Configures the gpio_pin as input/ouput, sets the drivestrength,voltage,and as
//   interrupt selection(if appicable).
//
//  Parameters:
//      gpio_pin
//          [in] Functional pin name
//
//      gpio_cfg
//          [in] Structure to configure the pin as input/ouput, interrupt selection.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------

BOOL DDKGpioConfig(DDK_IOMUX_PIN gpio_pin,
                        DDK_GPIO_CFG gpio_cfg)
{
    BOOL rc = FALSE;

    UINT32 BankNo = DDK_GPIO_GET_BANK(gpio_pin);
    UINT32 bit = DDK_GPIO_GET_BIT(gpio_pin);

    if(gpio_pin > DDK_IOMUX_GPIO_MAX_ID)
    {
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }

    if(gpio_cfg.DDK_PIN_IO == DDK_GPIO_OUTPUT)
    {
        switch(BankNo)
        {
            case 0:  
                HW_PINCTRL_DOE0_SET(bit); 
                break;
            case 1:  
                HW_PINCTRL_DOE1_SET(bit); 
                break;
            case 2:  
                HW_PINCTRL_DOE2_SET(bit); 
                break;
            case 3:  
                HW_PINCTRL_DOE3_SET(bit); 
                break;      
            case 4:  
                HW_PINCTRL_DOE4_SET(bit); 
                break;                  
            default: 
                goto cleanUp;
        } 
    }
    else
    {   
        // Disable output
        switch(BankNo)
        {
            case 0:  
                HW_PINCTRL_DOE0_CLR(bit); 
                break;
            case 1:  
                HW_PINCTRL_DOE1_CLR(bit); 
                break;
            case 2:  
                HW_PINCTRL_DOE2_CLR(bit); 
                break;
            case 3:  
                HW_PINCTRL_DOE3_CLR(bit); 
                break;
            case 4:  
                HW_PINCTRL_DOE4_CLR(bit); 
                break;                
            default: 
                goto cleanUp;

        }   

        if(!gpio_cfg.DDK_PIN_IRQ_CAPABLE)
        {
            // Disable interrupts until reconfigured
            switch(BankNo)
            {
                case 0:  
                    HW_PINCTRL_PIN2IRQ0_CLR(bit);
                    HW_PINCTRL_IRQEN0_CLR(bit);
                    break;
                case 1:  
                    HW_PINCTRL_PIN2IRQ1_CLR(bit);
                    HW_PINCTRL_IRQEN1_CLR(bit);
                    break;
                case 2:  
                    HW_PINCTRL_PIN2IRQ2_CLR(bit);
                    HW_PINCTRL_IRQEN2_CLR(bit);
                    break;
                case 3:  
                    HW_PINCTRL_PIN2IRQ3_CLR(bit);
                    HW_PINCTRL_IRQEN3_CLR(bit);
                    break;
                case 4:  
                    HW_PINCTRL_PIN2IRQ4_CLR(bit);
                    HW_PINCTRL_IRQEN4_CLR(bit);
                    break;
                default: 
                    goto cleanUp;
            }
        }
        else
        {
            switch(BankNo)
            {
                case 0:  
                    HW_PINCTRL_IRQSTAT0_CLR(bit);
                    HW_PINCTRL_PIN2IRQ0_SET(bit);
                    break;
                case 1:  
                    HW_PINCTRL_IRQSTAT1_CLR(bit);
                    HW_PINCTRL_PIN2IRQ1_SET(bit);
                    break;
                case 2:  
                    HW_PINCTRL_IRQSTAT2_CLR(bit);
                    HW_PINCTRL_PIN2IRQ2_SET(bit);
                    break;
                case 3:  
                    HW_PINCTRL_IRQSTAT3_CLR(bit);
                    HW_PINCTRL_PIN2IRQ3_SET(bit);
                    break;
                case 4:  
                    HW_PINCTRL_IRQSTAT4_CLR(bit);
                    HW_PINCTRL_PIN2IRQ4_SET(bit);
                    break;
                default: 
                    goto cleanUp;
            }
        }
        
        // Set interrupt level(level/edge trigger)
        if(gpio_cfg.DDK_PIN_IRQ_LEVEL)
        {
            switch( BankNo )
            {
                case 0:  
                    HW_PINCTRL_IRQLEVEL0_SET(bit); 
                    break;
                case 1:  
                    HW_PINCTRL_IRQLEVEL1_SET(bit); 
                    break;
                case 2:  
                    HW_PINCTRL_IRQLEVEL2_SET(bit); 
                    break;
                case 3:  
                    HW_PINCTRL_IRQLEVEL3_SET(bit); 
                    break;
                case 4:  
                    HW_PINCTRL_IRQLEVEL4_SET(bit); 
                    break;                    
                default: 
                    goto cleanUp;
            }
        } 
        else 
        {
            switch( BankNo )
            {
                case 0:  
                    HW_PINCTRL_IRQLEVEL0_CLR(bit); 
                    break;
                case 1:  
                    HW_PINCTRL_IRQLEVEL1_CLR(bit); 
                    break;
                case 2:  
                    HW_PINCTRL_IRQLEVEL2_CLR(bit); 
                    break;
                case 3:  
                    HW_PINCTRL_IRQLEVEL3_CLR(bit); 
                    break;
                case 4:  
                    HW_PINCTRL_IRQLEVEL4_CLR(bit); 
                    break;
                default: 
                    goto cleanUp;
            }
        }
        
        // Set interrupt Polarity
        if( gpio_cfg.DDK_PIN_IRQ_POLARITY)
        {
            switch(BankNo)
            {
                case 0:  
                    HW_PINCTRL_IRQPOL0_SET(bit); 
                    break;
                case 1:  
                    HW_PINCTRL_IRQPOL1_SET(bit); 
                    break;
                case 2:  
                    HW_PINCTRL_IRQPOL2_SET(bit); 
                    break;
                case 3:  
                    HW_PINCTRL_IRQPOL3_SET(bit); 
                    break;
                case 4:  
                    HW_PINCTRL_IRQPOL4_SET(bit); 
                    break;
                default: 
                    goto cleanUp;
            }
        } 
        else 
        {
            switch(BankNo)
            {
                case 0:  
                    HW_PINCTRL_IRQPOL0_CLR(bit); 
                    break;
                case 1:  
                    HW_PINCTRL_IRQPOL1_CLR(bit); 
                    break;
                case 2:  
                    HW_PINCTRL_IRQPOL2_CLR(bit); 
                    break;
                case 3:  
                    HW_PINCTRL_IRQPOL3_CLR(bit); 
                    break;
                case 4:  
                    HW_PINCTRL_IRQPOL4_CLR(bit); 
                    break;
                default: 
                    goto cleanUp;
            }
        }

        //Set interrupt src(Enable/Disable)
        if( gpio_cfg.DDK_PIN_IRQ_ENABLE)
        {
            switch(BankNo)
            {
                case 0:  
                    HW_PINCTRL_IRQEN0_SET(bit); 
                    break;
                case 1:  
                    HW_PINCTRL_IRQEN1_SET(bit); 
                    break;
                case 2:  
                    HW_PINCTRL_IRQEN2_SET(bit); 
                    break;
                case 3:  
                    HW_PINCTRL_IRQEN3_SET(bit); 
                    break; 
                case 4:  
                    HW_PINCTRL_IRQEN4_SET(bit); 
                    break;
                default: 
                    goto cleanUp;
            }      
        } 
        else 
        {
            switch(BankNo)
            {
                case 0:  
                    HW_PINCTRL_IRQEN0_CLR(bit); 
                    break;
                case 1:  
                    HW_PINCTRL_IRQEN1_CLR(bit); 
                    break;
                case 2:  
                    HW_PINCTRL_IRQEN2_CLR(bit); 
                    break;
                case 3:  
                    HW_PINCTRL_IRQEN3_CLR(bit); 
                    break;
                case 4:  
                    HW_PINCTRL_IRQEN4_CLR(bit); 
                    break;
                default: 
                    goto cleanUp;
            }       
       }
    }
    rc = TRUE;

cleanUp:

    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKGpioEnableDataPin
//
//  set the value in the register bit to drive on the gpio_pin.
//
//  Parameters:
//      pin
//          [in] Functional pin name
//
//      data
//          [in] data to be written to the pin.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioEnableDataPin(DDK_IOMUX_PIN pin,UINT32 data)
{
    BOOL rc = FALSE;
    
    UINT32 BankNo   = DDK_GPIO_GET_BANK(pin);
    UINT32 bit      = DDK_GPIO_GET_BIT(pin);

    if (pin > DDK_IOMUX_GPIO_MAX_ID)
    {
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }
       
    if(data)
    {
        switch( BankNo )
        {
            case 0:
                HW_PINCTRL_DOE0_SET(bit);
                break;
            case 1:
                HW_PINCTRL_DOE1_SET(bit);
                break;
            case 2:
                HW_PINCTRL_DOE2_SET(bit);
                break;
            case 3:
                HW_PINCTRL_DOE3_SET(bit);
                break;
            case 4:
                HW_PINCTRL_DOE4_SET(bit);
                break;
            default:
                goto cleanUp;
        }
    }
    else
    {
        switch( BankNo )
        {
            case 0:
                HW_PINCTRL_DOE0_CLR(bit);
                break;
            case 1:
                HW_PINCTRL_DOE1_CLR(bit);
                break;
            case 2:
                HW_PINCTRL_DOE2_CLR(bit);
                break;
            case 3:
                HW_PINCTRL_DOE3_CLR(bit);
                break;
            case 4:
                HW_PINCTRL_DOE4_CLR(bit);
                break;
            default:
                goto cleanUp;
        }
    }
    rc = TRUE;
    
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKGpioWriteDataPin
//
//  set the value in the register bit to drive on the gpio_pin.
//
//  Parameters:
//      pin
//          [in] Functional pin name
//
//      data
//          [in] data to be written to the pin.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioWriteDataPin(DDK_IOMUX_PIN pin,UINT32 data)
{
    BOOL rc = FALSE;

    UINT32 BankNo   = DDK_GPIO_GET_BANK(pin);
    UINT32 bit      = DDK_GPIO_GET_BIT(pin);

    if (pin > DDK_IOMUX_GPIO_MAX_ID)
    {
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }
    
    if(data)
    {
        switch(BankNo)
        {
            case 0:
                HW_PINCTRL_DOUT0_SET(bit);
                break;
            case 1:
                HW_PINCTRL_DOUT1_SET(bit);
                break;
            case 2:
                HW_PINCTRL_DOUT2_SET(bit);
                break;
            case 3:
                HW_PINCTRL_DOUT3_SET(bit);
                break;
            case 4:
                HW_PINCTRL_DOUT4_SET(bit);
                break;
            default:
                goto cleanUp;
        }
    }
    else
    {
        switch( BankNo )
        {
            case 0:
                HW_PINCTRL_DOUT0_CLR(bit);
                break;
            case 1:
                HW_PINCTRL_DOUT1_CLR(bit);
                break;
            case 2:
                HW_PINCTRL_DOUT2_CLR(bit);
                break;
            case 3:
                HW_PINCTRL_DOUT3_CLR(bit);
                break;
            case 4:
                HW_PINCTRL_DOUT4_CLR(bit);
                break;
            default:
                goto cleanUp;
        }
    }
    rc = TRUE;
    
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKGpioReadDataPin
//
//  Reads the GPIO port data from the specified pin.
//
//  Parameters:
//      pin
//          [in] GPIO pin [0-31].
//
//      pData
//          [out] Points to buffer for data read.  Data will be shifted to LSB.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------
BOOL DDKGpioReadDataPin(DDK_IOMUX_PIN pin, UINT32 *pData)
{
    BOOL rc = FALSE;
    UINT32 BankNo   = DDK_GPIO_GET_BANK(pin);
    
    if (pin > DDK_IOMUX_GPIO_MAX_ID)
    {
        DBGCHK((_T("CSPDDK")), FALSE);
        goto cleanUp;
    }

    switch(BankNo)
    {
        case 0:
            *pData = HW_PINCTRL_DIN0_RD();
            break;
        case 1:
            *pData = HW_PINCTRL_DIN1_RD();
            break;
        case 2:
            *pData = HW_PINCTRL_DIN2_RD();
            break;
        case 3:
            *pData = HW_PINCTRL_DIN3_RD();
            break;
        case 4:
            *pData = HW_PINCTRL_DIN4_RD();
            break;
        default:
            goto cleanUp;
    }
    *pData = (*pData >> DDK_GPIO_GET_PIN(pin)) & 0x1;
    rc = TRUE;
    
cleanUp:
    return rc;
}

//-----------------------------------------------------------------------------
//
//  Function: DDKGpioReadIntrPin
//
//  Reads Pin for the interrupt status.
//
//  Parameters:
//      pin
//          [in] Functional pin name
//
//      pData
//          [out] pointer to the Pin data read.
//
//  Returns:
//      Returns TRUE if successful, otherwise returns FALSE.
//
//-----------------------------------------------------------------------------

BOOL DDKGpioReadIntrPin(DDK_IOMUX_PIN pin, UINT32 *pData)
{
    BOOL rc = FALSE;

    UINT32 BankNo = DDK_GPIO_GET_BANK(pin);

    switch( BankNo )
    {
        case 0:  
            *pData=HW_PINCTRL_IRQSTAT0_RD();
            break;
        case 1:  
            *pData=HW_PINCTRL_IRQSTAT1_RD();
            break;
        case 2:  
            *pData=HW_PINCTRL_IRQSTAT2_RD();
            break;
        case 3:  
            *pData=HW_PINCTRL_IRQSTAT3_RD();
            break;
        case 4:  
            *pData=HW_PINCTRL_IRQSTAT4_RD();
            break;
        default: 
            goto cleanUp;
    }

    *pData = (*pData >> DDK_GPIO_GET_PIN(pin)) & 0x1;    
    rc = TRUE;
    
cleanUp:
    return rc;

}


//-----------------------------------------------------------------------------
//
//  Function: DDKGpioClearIntrPin
//
//  Clear the interrupt status.
//
//  Parameters:
//      pin
//          [in] Functional pin name
//
//  Returns:
//      None.
//
//-----------------------------------------------------------------------------

VOID DDKGpioClearIntrPin(DDK_IOMUX_PIN gpio_pin)
{
    UINT32 BankNo = DDK_GPIO_GET_BANK(gpio_pin);
    UINT32 bit =  DDK_GPIO_GET_BIT(gpio_pin); 

    switch( BankNo )
    {
        case 0: 
            HW_PINCTRL_IRQSTAT0_CLR(bit);
            break;
        case 1:  
            HW_PINCTRL_IRQSTAT1_CLR(bit);
            break;
        case 2:  
            HW_PINCTRL_IRQSTAT2_CLR(bit); 
            break;
        case 3: 
            HW_PINCTRL_IRQSTAT3_CLR(bit);
            break;
        case 4: 
            HW_PINCTRL_IRQSTAT4_CLR(bit);
            break;
        default: 
            break;
    } 

}

//-------------------------------------------------------------
// CS&ZHL MAY-17-2012: supporting GPIO interrupt
//-------------------------------------------------------------
VOID DDKGpioIntrruptEnable(DDK_IOMUX_PIN gpio_pin)
{
    UINT32 BankNo = DDK_GPIO_GET_BANK(gpio_pin);
    UINT32 bit =  DDK_GPIO_GET_BIT(gpio_pin); 

    switch( BankNo )
    {
        case 0: 
            HW_PINCTRL_IRQEN0_SET(bit);
            break;
        case 1:  
            HW_PINCTRL_IRQEN1_SET(bit);
            break;
        case 2:  
            HW_PINCTRL_IRQEN2_SET(bit); 
            break;
        case 3: 
            HW_PINCTRL_IRQEN3_SET(bit);
            break;
        case 4: 
            HW_PINCTRL_IRQEN3_SET(bit);
            break;
        default: 
            break;
    } 
}

VOID DDKGpioIntrruptDisable(DDK_IOMUX_PIN gpio_pin)
{
    UINT32 BankNo = DDK_GPIO_GET_BANK(gpio_pin);
    UINT32 bit =  DDK_GPIO_GET_BIT(gpio_pin); 

    switch( BankNo )
    {
        case 0: 
            HW_PINCTRL_IRQEN0_CLR(bit);
            break;
        case 1:  
            HW_PINCTRL_IRQEN1_CLR(bit);
            break;
        case 2:  
            HW_PINCTRL_IRQEN2_CLR(bit); 
            break;
        case 3: 
            HW_PINCTRL_IRQEN3_CLR(bit);
            break;
        case 4: 
            HW_PINCTRL_IRQEN3_CLR(bit);
            break;
        default: 
            break;
    } 
}
// end of CS&ZHL MAY-17-2012: supporting GPIO interrupt
