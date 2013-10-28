//
// Copyright (c) Texas Instruments Incorporated 2010. All Rights Reserved.
//
//------------------------------------------------------------------------------

//! \file Emac_Phy.c
//! \brief Contains EMAC controller PHY related functions.
//! 
//! This source file contains Intel PHY related init,read and write routines.
//! These are being called for link handling.
//! 
//! \version  1.00 Aug 22nd 2006 File Created

// Includes
#include <Ndis.h>
#include "Emac_Adapter.h"
#include "Emac_Queue.h"

PEMACMDIOREGS f_pMdioRegs;
PEMACREGS	  f_pEmacRegs;	

//========================================================================
//!  \fn UINT32 ReadPhyRegister( UINT32 PhyAddr, UINT32 RegNum)
//!  \brief Read a Phy register via MDIO inteface.
//!  \param PhyAddr UINT32 Bit mask of link
//!  \param RegNum  UINT32 Registe number.
//!  \return UINT32  Register value.
//========================================================================
UINT32 ReadPhyRegister(UINT32 PhyAddr, UINT32 RegNum )
{
    /* Wait for any previous command to complete */ 
    while ( (f_pMdioRegs->USERACCESS0 & MDIO_USERACCESS0_GO) != 0 );

    f_pMdioRegs->USERACCESS0 =((MDIO_USERACCESS0_GO) |(MDIO_USERACCESS0_WRITE_READ)|
                                ((RegNum & 0x1F) << 21) | ((PhyAddr & 0x1F) << 16));

    /* Wait for command to complete */
    while ( (f_pMdioRegs->USERACCESS0 & MDIO_USERACCESS0_GO) != 0 );

    return (f_pMdioRegs->USERACCESS0 & 0xFFFF);
}

//========================================================================
//!  \fn void    WritePhyRegister( UINT32 PhyAddr,UINT32 RegNum,UINT32 Data )
//!  \brief Write to a Phy register via MDIO inteface
//!  \param PhyAddr UINT32 Bit mask of link.
//!  \param RegNum  UINT32 Register number to be written.
//!  \param Data   UINT32 Data to be written.
//!  \return VOID Returns none.
//========================================================================
void WritePhyRegister(UINT32 PhyAddr, UINT32 RegNum, UINT32 Data)
{
    /* Wait for any previous command to complete */ 
    while ( (f_pMdioRegs->USERACCESS0 & MDIO_USERACCESS0_GO) != 0 );

    f_pMdioRegs->USERACCESS0 =((MDIO_USERACCESS0_GO) | (MDIO_USERACCESS0_WRITE_WRITE) |
                                ((RegNum & 0x1F) << 21) | ((PhyAddr & 0x1F) << 16) |
                                (Data & 0xFFFF));

}

//========================================================================
//!  \fn VOID LinkChangeIntrHandler(MINIPORT_ADAPTER* Adapter)
//!  \brief Handles link change interrupt from MiniporthandleInterrrupt
//!  \param pAdapter PMINIPORT_ADAPTER Pointer to receive pointer to our adapter
//!  \return NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE
//========================================================================
#define MII_PHY_CONFIG_REG          22
#define MII_PHY_STATUS_REG          26
#define PHY_TX_CLK_EN		     	(1 << 5)

#define PHY_SPEED_1000BASET			0x0200
#define PHY_SPEED_MASK				0x0300
#define PHY_DUPLEX_MASK				0x0080
#define PHY_FULL_DUPLEX				0x0080

#define EMAC_MACCONTROL_GIG_ENABLE  (1 << 7)
#define EMAC_MACCONTROL_GIGFORCE    (1 << 17)

VOID LinkChangeIntrHandler( MINIPORT_ADAPTER* Adapter )
{
	UINT32 regPhyVal;
	UINT32 macCtrl;
	static int enable_phy_tx_clk = 1;
    f_pMdioRegs = Adapter->m_pMdioRegsBase;
	f_pEmacRegs = Adapter->m_EmacBaseAddr;
    
RETAILMSG(1, (L"LinkChangeIntrHandler-->m_ActivePhy = 0x%X\r\n", Adapter->m_ActivePhy));
    DEBUGMSG(DBG_FUNC, (L"LinkChangeIntrHandler-->m_ActivePhy = 0x%X\r\n", Adapter->m_ActivePhy));
    /* Extract the Link status for the active PHY */
    if(0 == (f_pMdioRegs->LINK & (BIT(0) << Adapter->m_ActivePhy)))
    {
        Adapter->m_LinkStatus = DOWN;
         /* Link was active last time, now it is down. */
        NdisMIndicateStatus(Adapter->m_AdapterHandle, NDIS_STATUS_MEDIA_DISCONNECT,(PVOID)0,0);
        NdisMIndicateStatusComplete(Adapter->m_AdapterHandle);   
        DEBUGMSG(DBG_INFO, (L"LinkChangeIntrHandler NDIS_STATUS_MEDIA_DISCONNECT\r\n"));
    } 
    else 
    {   
        Adapter->m_LinkStatus = UP;

		if (enable_phy_tx_clk != 0){
			UINT32 uiRet; 
			enable_phy_tx_clk = 0;
			// enable output of 1000Base-T Phy TX_CLK pin  
			uiRet = ReadPhyRegister(Adapter->m_ActivePhy, MII_PHY_CONFIG_REG);
			uiRet |= PHY_TX_CLK_EN;
			WritePhyRegister(Adapter->m_ActivePhy, MII_PHY_CONFIG_REG,uiRet);
			uiRet = ReadPhyRegister(Adapter->m_ActivePhy, MII_PHY_CONFIG_REG);
		}

        /* Link was active last time, now it is up. */
		
		regPhyVal = ReadPhyRegister(Adapter->m_ActivePhy, MII_PHY_STATUS_REG);
		macCtrl   = f_pEmacRegs->MACCONTROL;
		if (regPhyVal & PHY_DUPLEX_MASK)
			macCtrl |= EMAC_MACCONTROL_FULLDUPLEX_ENABLE;
		else
			macCtrl &= ~EMAC_MACCONTROL_FULLDUPLEX_ENABLE;
		
		if ((regPhyVal & PHY_SPEED_MASK) == PHY_SPEED_1000BASET)
			macCtrl |= EMAC_MACCONTROL_GIG_ENABLE | EMAC_MACCONTROL_GIGFORCE;
		else
			macCtrl &= ~(EMAC_MACCONTROL_GIG_ENABLE | EMAC_MACCONTROL_GIGFORCE);
			
		f_pEmacRegs->MACCONTROL = macCtrl;
		
        NdisMIndicateStatus(Adapter->m_AdapterHandle, NDIS_STATUS_MEDIA_CONNECT, (PVOID)0,0);
        NdisMIndicateStatusComplete(Adapter->m_AdapterHandle);
        DEBUGMSG(DBG_INFO, (L"LinkChangeIntrHandler NDIS_STATUS_MEDIA_CONNECT\r\n"));
    } 
    /* Acknowledge interrupt */
    f_pMdioRegs->LINKINTMASKED = 0x1;
}
