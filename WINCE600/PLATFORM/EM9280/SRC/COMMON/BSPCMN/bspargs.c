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
//  Copyright (C) 2007-2010, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//------------------------------------------------------------------------------

#include <bsp.h>

PDDK_CLK_CONFIG g_pDdkClkConfig;

//------------------------------------------------------------------------------
// Local Variables
const UINT32 PfdPllFreqs[] =
{
    //|-----------+-----------|
    //|  PLL Freq | PFD Value |
    //|-----------+-----------|
    480000000,    // 18 (MIN_PFD_VALUE)
    454736842,    // 19
    432000000,    // 20
    411428571,    // 21
    392727272,    // 22
    375652173,    // 23
    360000000,    // 24
    345600000,    // 25
    332307692,    // 26
    320000000,    // 27
    308571428,    // 28
    297931034,    // 29
    288000000,    // 30
    278709677,    // 31
    270000000,    // 32
    261818181,    // 33
    254117647,    // 34
    246857142     // 35 (MAX_PFD_VALUE)
};

//------------------------------------------------------------------------------
// Functions


//------------------------------------------------------------------------------
//
//  Function:   OALBspArgsInit
//
//  This function initializes the parameters of the global BSP args structure.
//
//  Parameters:
//      pBSPArgs
//          [out] Points to BSP arguments structure to be updated.
//
//  Returns:
//      TRUE if boot successfully, otherwise returns FALSE.
//
//------------------------------------------------------------------------------
BOOL OALBspArgsInit(VOID)
{
    UINT32 pllctrl0,clkseq,frac0,frac1;
    UINT32 iofrac,cpufrac,emifrac,pixfrac,hsadcfrac,gpmifrac;
    UINT32 hbus,hbusdiv,xbus,xbusdiv;
    UINT32 hsadc,hsadcdiv,ssp0,ssp1,ssp2,ssp3,ssp0div,ssp1div,ssp2div,ssp3div;
    UINT32 etm,etmdiv,cpu,cpudiv,xtal,xtaldiv,gpmi,gpmidiv,emi,emidiv;
    UINT32 pix,pixdiv,saif0,saif0div,saif1,saif1div,spdif;     
    DDK_CLOCK_SIGNAL index;
    DDK_CLOCK_BAUD_SOURCE baudSrc;
    DDK_DVFC_DOMAIN domain;
    DDK_DVFC_SETPOINT setpoint;
    PVOID pv_HWregCLKCTRL = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_CLKCTRL);

    g_pDdkClkConfig = (PDDK_CLK_CONFIG) IMAGE_WINCE_DDKCLK_RAM_UA_START;
    if (g_pDdkClkConfig == NULL)
    {
        return FALSE;
    }
    
    //
    for (index = 0; index < DDK_CLOCK_SIGNAL_ENUM_END; index++)
    {
        // Initialize clock references
        g_pDdkClkConfig->clockFreq[index] = 0;
    }

    for (index = 0; index < DDK_CLOCK_GATE_ENUM_END; index++)
    {
        // Initialize clock  references
        g_pDdkClkConfig->clockRefCount[index] = 0;
        
        // Initialize clock tree root references
        g_pDdkClkConfig->root[index] = 0;
    }
    
    for (baudSrc = 0; baudSrc < DDK_CLOCK_BAUD_SOURCE_ENUM_END; baudSrc++)
    {
        // Initialize root clock references
        g_pDdkClkConfig->rootRefCount[baudSrc] = 0;
    }

    //
    for (domain = 0; domain < DDK_DVFC_DOMAIN_ENUM_END; domain++)
    {
        for (setpoint = 0; setpoint < DDK_DVFC_SETPOINT_ENUM_END; setpoint++)
        {
            // Initialize setpoint requests
            g_pDdkClkConfig->setpointReqCount[domain][setpoint] = 0;
        }

        // Initialize load tracking requests
        g_pDdkClkConfig->setpointLoad[domain] = DDK_DVFC_SETPOINT_ENUM_END-1;
    }
    
    // Initialize DVFC state variables
    g_pDdkClkConfig->bDvfcActive = FALSE;
    g_pDdkClkConfig->bSetpointPending = FALSE;
    g_pDdkClkConfig->setpointCur[DDK_DVFC_DOMAIN_CPU] = DDK_DVFC_SETPOINT_HIGH;
    g_pDdkClkConfig->setpointMin[DDK_DVFC_DOMAIN_CPU] = DDK_DVFC_SETPOINT_LOW;
    g_pDdkClkConfig->setpointMax[DDK_DVFC_DOMAIN_CPU] = DDK_DVFC_SETPOINT_HIGH;

    //XTAL is always 24MHz
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] = XTAL_24MHZ;

    pllctrl0 = HW_CLKCTRL_PLL0CTRL0_RD();

    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_PLL0] = PLL_480MHZ;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_PLL1] = PLL_480MHZ;

    //UTMI0 480M
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_UTMI0] = PLL_480MHZ;
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_UTMI0_CLK480M_CLK] = DDK_CLOCK_BAUD_SOURCE_PLL0;

    //UTMI1 480M
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_UTMI1] = PLL_480MHZ;
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_UTMI1_CLK480M_CLK] = DDK_CLOCK_BAUD_SOURCE_PLL1;   

    //Init Enet root Source 
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_ENET_CLK] = DDK_CLOCK_BAUD_SOURCE_PLL2;  

    //REF_PLL
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PLL] = 
                     g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_PLL0];

    frac0 = HW_CLKCTRL_FRAC0_RD();
    frac1 = HW_CLKCTRL_FRAC1_RD();

    //REF_CPU
    cpufrac = (frac0& BM_CLKCTRL_FRAC0_CPUFRAC) >> BP_CLKCTRL_FRAC0_CPUFRAC;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_CPU] = PfdPllFreqs[cpufrac-MIN_PFD_VALUE];    

    //REF_EMI
    emifrac = (frac0 & BM_CLKCTRL_FRAC0_EMIFRAC) >> BP_CLKCTRL_FRAC0_EMIFRAC;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_EMI] = PfdPllFreqs[emifrac-MIN_PFD_VALUE];        

    //REF_IO0
    iofrac = (frac0 & BM_CLKCTRL_FRAC0_IO0FRAC) >> BP_CLKCTRL_FRAC0_IO0FRAC;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO0] = PfdPllFreqs[iofrac-MIN_PFD_VALUE]; 
    
    //REF_IO1
    iofrac = (frac0 & BM_CLKCTRL_FRAC0_IO1FRAC) >> BP_CLKCTRL_FRAC0_IO1FRAC;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO1] = PfdPllFreqs[iofrac-MIN_PFD_VALUE]; 

    //REF_PIX
    pixfrac = (frac1 & BM_CLKCTRL_FRAC1_PIXFRAC) >> BP_CLKCTRL_FRAC1_PIXFRAC;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PIX] = PfdPllFreqs[pixfrac-MIN_PFD_VALUE];    

    //REF_HSADC
    hsadcfrac = (frac1 & BM_CLKCTRL_FRAC1_HSADCFRAC) >> BP_CLKCTRL_FRAC1_HSADCFRAC;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_HSADC] = PfdPllFreqs[hsadcfrac-MIN_PFD_VALUE];    

    //REF_GPMI
    gpmifrac = (frac1 & BM_CLKCTRL_FRAC1_GPMIFRAC) >> BP_CLKCTRL_FRAC1_GPMIFRAC;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_GPMI] = PfdPllFreqs[gpmifrac-MIN_PFD_VALUE];    


    clkseq = HW_CLKCTRL_CLKSEQ_RD();

    //ETM Clock 
    etm = HW_CLKCTRL_ETM_RD();
    etmdiv = etm & BM_CLKCTRL_ETM_DIV;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_ETM) != 0)
    {
       baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
       g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_ETM] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / etmdiv;       
    }
    else 
    {
        baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_CPU;
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_ETM] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_CPU] / etmdiv;
    }    
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_ETM_CLK] = baudSrc;        
    
    //CPU clock CLK_P
    cpu = HW_CLKCTRL_CPU_RD();
    cpudiv = cpu & BM_CLKCTRL_CPU_DIV_CPU;
    xtaldiv = (cpu & BM_CLKCTRL_CPU_DIV_XTAL) >> BP_CLKCTRL_CPU_DIV_XTAL;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_CPU) != 0)
    {
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_P_CLK] = 
              g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / xtaldiv;

    }
    else 
    {
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_P_CLK] = 
              g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_CPU] / cpudiv;        

    }    

    //APBH Clock
    hbus = HW_CLKCTRL_HBUS_RD();
    hbusdiv = hbus & BM_CLKCTRL_HBUS_DIV;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_H_CLK] = 
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_P_CLK] / hbusdiv;

    //APBX Clock
    xbus = HW_CLKCTRL_XBUS_RD();
    xbusdiv = xbus & BM_CLKCTRL_XBUS_DIV;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_X_CLK] = 
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / xbusdiv;

    //EMI
    emi = HW_CLKCTRL_EMI_RD();
    emidiv = emi & BM_CLKCTRL_EMI_DIV_EMI;
    xtaldiv = (emi & BM_CLKCTRL_EMI_DIV_XTAL) >> BP_CLKCTRL_EMI_DIV_XTAL;   
    baudSrc =  DDK_CLOCK_BAUD_SOURCE_PLL0;
    if((emi & BM_CLKCTRL_EMI_SYNC_MODE_EN) != 0)
    {
        //Sync with APBH clock
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_EMI] = 
                g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_H_CLK];
    }   
    else
    {
        if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_EMI) != 0)
        {
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_EMI] = 
                       g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / xtaldiv;  
           baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
        }
        else 
        {
            g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_EMI] = 
                       g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_EMI] / emidiv;
        } 
    }  
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_EMI_CLK] = baudSrc;
        
    //SSP0
    ssp0 = HW_CLKCTRL_SSP0_RD();
    ssp0div = ssp0 & BM_CLKCTRL_SSP0_DIV;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_SSP0) != 0)
    {
       baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
       g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP0] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / ssp0div;       
    }
    else 
    {
        baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_IO0;
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP0] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO0] / ssp0div;
    }
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_SSP0_CLK] = baudSrc;        
    
    //SSP1
    ssp1 = HW_CLKCTRL_SSP1_RD();
    ssp1div = ssp1 & BM_CLKCTRL_SSP1_DIV;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_SSP1) != 0)
    {
       baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
       g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP1] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / ssp1div;       
    }
    else 
    {
        baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_IO0;
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP1] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO0] / ssp1div;
    }
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_SSP1_CLK] = baudSrc;        

    //SSP2
    ssp2 = HW_CLKCTRL_SSP2_RD();
    ssp2div = ssp2 & BM_CLKCTRL_SSP2_DIV;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_SSP2) != 0)
    {
       baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
       g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP2] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / ssp2div;       
    }
    else 
    {
        baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_IO1;
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP2] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO1] / ssp2div;
    }
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_SSP2_CLK] = baudSrc;        
        
    //SSP3
    ssp3 = HW_CLKCTRL_SSP3_RD();
    ssp3div = ssp3 & BM_CLKCTRL_SSP3_DIV;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_SSP3) != 0)
    {
       baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
       g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP3] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / ssp3div;       
    }
    else 
    {
        baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_IO1;
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP3] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO1] / ssp3div;
    }
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_SSP3_CLK] = baudSrc;        
    

    //GPMI
    gpmi = HW_CLKCTRL_GPMI_RD(); 
    gpmidiv = gpmi & BM_CLKCTRL_GPMI_DIV;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_GPMI) != 0)
    {
       baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
       g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_GPMI] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / gpmidiv;       
    }
    else 
    {
        baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_GPMI;
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_GPMI] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_GPMI] / gpmidiv;
    }
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_GPMI_CLK] = baudSrc;

    //HSADC
    hsadc = HW_CLKCTRL_HSADC_RD(); 
    hsadcdiv = hsadc & BM_CLKCTRL_HSADC_FREQDIV >> BP_CLKCTRL_HSADC_FREQDIV;
    baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_HSADC;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_HSADC] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_HSADC] / (9 * (2 ^ hsadcdiv));

    g_pDdkClkConfig->root[DDK_CLOCK_GATE_HSADC_CLK] = baudSrc; 
    
    //PIX
    pix = HW_CLKCTRL_DIS_LCDIF_RD();
    pixdiv = pix & BM_CLKCTRL_DIS_LCDIF_DIV;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF) != 0)
    {
       baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
       g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_DIS_LCDIF] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / pixdiv;       
    }
    else 
    {
        baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_PIX;
        g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_DIS_LCDIF] = 
                   g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PIX] / pixdiv;
    } 
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_DIS_LCDIF_CLK] = baudSrc;       

        
    //SAIF0
    saif0 = HW_CLKCTRL_SAIF0_RD();        
    saif0div = saif0 & BM_CLKCTRL_SAIF0_DIV;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_SAIF0) != 0)
    {
       baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
       //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SAIF0] = 
                   //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / saif0div;       
    }
    else 
    {
        baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_PLL;
        //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SAIF0] = 
                   //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PLL] / saif0div;
    } 
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_SAIF0_CLK] = baudSrc;        

    //SAIF1
    saif1 = HW_CLKCTRL_SAIF1_RD();        
    saif1div = saif1 & BM_CLKCTRL_SAIF1_DIV;
    if((clkseq & BM_CLKCTRL_CLKSEQ_BYPASS_SAIF1) != 0)
    {
       baudSrc =  DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
       //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SAIF1] = 
                   //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL] / saif1div;       
    }
    else 
    {
        baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_PLL;
        //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SAIF1] = 
                   //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PLL] / saif1div;
    } 
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_SAIF1_CLK] = baudSrc;        

    //SPDIF
    spdif = HW_CLKCTRL_SPDIF_RD();
    baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_PLL;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SPDIF] = SPDIF_120MHZ;
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_SPDIF_CLK] = baudSrc;        

    //CAN0, CAN1
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_FLEXCAN0] = XTAL_24MHZ;    
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_FLEXCAN1] = XTAL_24MHZ;  
    baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_FLEXCAN0_CLK] = baudSrc; 
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_FLEXCAN1_CLK] = baudSrc; 

    //XTAL
    xtal = HW_CLKCTRL_XTAL_RD(); 
    
    //UART     
    baudSrc = DDK_CLOCK_BAUD_SOURCE_REF_XTAL;
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_UART] = XTAL_24MHZ;
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_UART_CLK] = baudSrc; 

    //PWM  
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_PWM24M] = XTAL_24MHZ;
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_PWM24M_CLK] = baudSrc;    

    //TIMROT_CLK32K   
    g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_TIMROT32K] = TIMROT_CLK32KHZ;        
    g_pDdkClkConfig->root[DDK_CLOCK_GATE_TIMROT32K_CLK] = baudSrc;               

    return TRUE;
}


//------------------------------------------------------------------------------
//
//  Function:   OALBspArgsPrint
//
//  This function prints the parameters of the global BSP args structure.
//
//  Parameters:
//      g_pDdkClkConfig
//          [in] Points to BSP arguments structure to be printed.
//
//  Returns:
//      None
//
//------------------------------------------------------------------------------
VOID OALBspArgsPrint(void)
{
    //PVOID pv_HWregCLKCTRL = (PVOID) OALPAtoUA(CSP_BASE_REG_PA_CLKCTRL);

    if (g_pDdkClkConfig == NULL)
    {
        goto cleanUp;
    }
/*
    OALMSG(OAL_INFO, (TEXT("  CLKSEQ = %x\r\n"),
           HW_CLKCTRL_CLKSEQ_RD()));

    OALMSG(OAL_INFO, (TEXT("  EMI = %x\r\n"),
           HW_CLKCTRL_EMI_RD()));
*/
    // Print system configuration
    OALMSG(OAL_INFO, (TEXT("BSP Clock Configuration:\r\n")));
    OALMSG(OAL_INFO, (TEXT("    PLL0        = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_PLL0]));
    OALMSG(OAL_INFO, (TEXT("    PLL1        = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_PLL1]));
    OALMSG(OAL_INFO, (TEXT("    PLL2        = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_PLL2]));
    OALMSG(OAL_INFO, (TEXT("    REF_CPU     = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_CPU]));
    OALMSG(OAL_INFO, (TEXT("    REF_EMI     = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_EMI]));
    OALMSG(OAL_INFO, (TEXT("    REF_IO0     = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO0]));
    OALMSG(OAL_INFO, (TEXT("    REF_IO1     = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_IO1]));    
    OALMSG(OAL_INFO, (TEXT("    REF_PIX     = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PIX]));
    OALMSG(OAL_INFO, (TEXT("    REF_HSADC   = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_HSADC]));
    OALMSG(OAL_INFO, (TEXT("    REF_GPMI    = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_GPMI]));
    OALMSG(OAL_INFO, (TEXT("    REF_PLL     = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_PLL]));
    OALMSG(OAL_INFO, (TEXT("    REF_XTAL    = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_XTAL]));
    OALMSG(OAL_INFO, (TEXT("    REF_ENET_PLL= %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_REF_ENET_PLL]));
    OALMSG(OAL_INFO, (TEXT("    P_CLK       = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_P_CLK]));
    OALMSG(OAL_INFO, (TEXT("    H_CLK       = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_H_CLK]));
    OALMSG(OAL_INFO, (TEXT("    X_CLk       = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_X_CLK]));
    OALMSG(OAL_INFO, (TEXT("    ETM         = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_ETM]));
    OALMSG(OAL_INFO, (TEXT("    EMI         = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_EMI]));
    OALMSG(OAL_INFO, (TEXT("    SSP0        = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP0]));
    OALMSG(OAL_INFO, (TEXT("    SSP1        = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP1]));
    OALMSG(OAL_INFO, (TEXT("    SSP2        = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP2]));
    OALMSG(OAL_INFO, (TEXT("    SSP3        = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SSP3]));
    OALMSG(OAL_INFO, (TEXT("    GPMI        = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_GPMI]));
    OALMSG(OAL_INFO, (TEXT("    HSADC       = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_HSADC]));    
    OALMSG(OAL_INFO, (TEXT("    LCDIF       = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_DIS_LCDIF]));
    //OALMSG(OAL_INFO, (TEXT("    SAIF0       = %10d Hz\r\n"),
           //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SAIF0]));
    //OALMSG(OAL_INFO, (TEXT("    SAIF1       = %10d Hz\r\n"),
           //g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SAIF1]));    
    OALMSG(OAL_INFO, (TEXT("    SPDIF       = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_SPDIF]));
    OALMSG(OAL_INFO, (TEXT("    UTMI0       = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_UTMI0]));
    OALMSG(OAL_INFO, (TEXT("    UTMI1       = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_UTMI1]));    
    OALMSG(OAL_INFO, (TEXT("    UART24MHZ   = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_UART]));
    OALMSG(OAL_INFO, (TEXT("    PWM24MHZ    = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_PWM24M]));
    OALMSG(OAL_INFO, (TEXT("    TIMROT32K   = %10d Hz\r\n"),
           g_pDdkClkConfig->clockFreq[DDK_CLOCK_SIGNAL_TIMROT32K]));


cleanUp:
    return;
}
