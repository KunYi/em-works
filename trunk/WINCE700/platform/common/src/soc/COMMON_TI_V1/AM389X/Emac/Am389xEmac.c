//========================================================================
//   Copyright 2006 Mistral Software Pvt Ltd.
//
//   Use of this software is controlled by the terms and conditions found
//   in the license agreement under which this software has been supplied.
//========================================================================

//! \file Am389xEmac.c
//! \brief EMAC ethernet controller routines.
//!
//! This source file contains the routines used for Initialization
//! of the EMAC Controller and the MDIO Module. The routines defined
//! in this source file is utilized by the bootloader and the OAL Modules.
//!
//! \version  1.00 Created on May 10th 2006

// Includes
#include <windows.h>
#include <oal_log.h>
#include <oal_memory.h>
#include <ethdbg.h>

#include "Am389xEmacRegs.h"

#ifdef DUMP_FRAME
    #define htons( value ) ((UINT16)((((UINT16)value) << 8) | (((UINT16)((UINT16)value)) >> 8)))
    #define ntohs( value ) htons( value )
    #define htonl( value ) ((((ULONG)value) << 24) | ((0x0000FF00UL & ((ULONG)value)) << 8) | ((0x00FF0000UL & ((ULONG)value)) >> 8) | (((ULONG)value) >> 24))
    #define ntohl( value ) htonl( value )
#endif

// Module level Global Variables


static PEMAC_REGS  f_pEmacRegs ;
static PEMAC_CTRL_REGS f_pEmacCtlRegs;
static PEMAC_MDIO_REGS  f_pMdioRegs ;
static volatile PEMAC_DESC f_pEmacRxDesc;
static volatile PEMAC_DESC f_pEmacTxDesc;
static volatile PEMAC_DESC f_pEmacRx_ActiveHead;
static volatile PEMAC_DESC f_pEmacRx_ActiveTail;
static UINT32 f_Link =0xFFFFFFFF;				// Global variable for Phy link status

// This address is for first PHy on Netra EVM
// We need to find a way to get it somehow from board configuration
static UINT32 f_PhyAddr = 1;  

static PUCHAR  f_pucPhyRxBuffer = (PUCHAR) EMAC_BUF_BASE;
static BOOL  f_InterruptMode = FALSE;	// Global variable to denote interrupt mode

#ifdef DUMP_FRAME
//========================================================================
static void DumpEtherFrame( BYTE* pFrame, WORD cwFrameLength )
{
//    int i,j;
    EdbgOutputDebugString( "Frame Buffer Address: 0x%X\r\n", pFrame );
    EdbgOutputDebugString( "To: %B:%B:%B:%B:%B:%B  From: %B:%B:%B:%B:%B:%B" \
                           "Type: 0x%H  Length: %u\r\n",
                           pFrame[0], pFrame[1], pFrame[2], pFrame[3], pFrame[4], pFrame[5],
                           pFrame[6], pFrame[7], pFrame[8], pFrame[9], pFrame[10], pFrame[11],
                           ntohs(*((UINT16 *)(pFrame + 12))), cwFrameLength );
#if 0
    for ( i = 0; i < cwFrameLength / 16; i++ ){
        for ( j = 0; j < 16; j++ ) {
            EdbgOutputDebugString( " %B", pFrame[i*16 + j] );
        }
        EdbgOutputDebugString( "\r\n" );
	}
    for ( j = 0; j < cwFrameLength % 16; j++ )
        EdbgOutputDebugString( " %B", pFrame[i*16 + j] );
#endif

    EdbgOutputDebugString( "\r\n" );
}

#endif

#if 1
void MyDumpFrame(BYTE* pbData, DWORD pwLength)
{
	DWORD j;
	OALMSGS(1,(L"----------------- %d\r\n", pwLength));

	for (j=0;j<pwLength;j++){
		OALMSGS(1,(L"%02X%s", pbData[j], ((j+1)&0xF)?L" ":L"\r\n"));
	}
	OALMSGS(1,(L"\r\n\n"));	
}
#endif

//========================================================================
static UINT32 AM389X_EmacMdioRead(
    UINT32 PhyAddr,     /* Bit mask of link */
    UINT32 RegNum       /* Registe number */
    )
{
    
    while ( (f_pMdioRegs->m_Useraccess0 & MDIO_USERACCESS0_GO) != 0 );
    f_pMdioRegs->m_Useraccess0 =((MDIO_USERACCESS0_GO) | ((RegNum & 0x1F) << 21) |
                                ((PhyAddr & 0x1F) << 16));
    while ( (f_pMdioRegs->m_Useraccess0 & MDIO_USERACCESS0_GO) != 0 );

    return (f_pMdioRegs->m_Useraccess0 & 0xFFFF);
}

//========================================================================
VOID AM389X_EmacMdioWrite(
    UINT32 PhyAddr,     /* Bit mask of link */
    UINT32 RegNum,      /* Register number to be written */
    UINT32 Data         /* Data to be written */
    )
{
    while ( (f_pMdioRegs->m_Useraccess0 & MDIO_USERACCESS0_GO) != 0 );

    f_pMdioRegs->m_Useraccess0 =((MDIO_USERACCESS0_GO) | (MDIO_USERACCESS0_WRITE_WRITE) |
                                ((RegNum & 0x1F) << 21) | ((PhyAddr & 0x1F) << 16) |
                                (Data & 0xFFFF));

}
//========================================================================
//!  \fn UINT32 AM389X_EmacGetLink( UINT32 PhyMask )
//!  \brief  Function whether link is present.
//!  \param  Phy_mask Mask of the links to be tested.
//!  \return UINT32 Returns the bit mask of link.
//========================================================================
UINT32 AM389X_EmacGetLink(
    UINT32 PhyMask      /* Mask of the links to be tested */
    )
{
    UINT32 ActPhy;          /* Phy  Bit mask to be probed */
    UINT32 LinkState = 0;   /* To maintain link state  */
    UINT32 PhyAddr   = 0;   /* Phy number to be probed */
//    UINT32 Config    ;   /* Configuration value to be written */
    UINT32 Regval=0;

    ActPhy =  (f_pMdioRegs->m_Alive & PhyMask);
    if ( (ActPhy != 0)){
        /* find the Phy number */
        while (ActPhy != 0) {
            while ( 0 == (ActPhy & (1 << 0)) ){
                PhyAddr++;
                ActPhy >>= 1;
            }

            /* Read the status register from Phy */
            Regval=AM389X_EmacMdioRead(PhyAddr, MII_STATUS_REG);
            LinkState =(Regval & PHY_LINK_STATUS);

            if ( 0 != LinkState   ){
                break;
            } else {
                /* If no link, go to next Phy. */
                ActPhy >>= 1;
                PhyAddr++;
            }
        }
    }
    if ((0 != LinkState) && (-1 == f_Link)){
        f_Link = 1 << PhyAddr;
		f_PhyAddr = PhyAddr;
    }

    return (LinkState);
}

VOID AM389X_EmacPowerOn(VOID)
{
    /*OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+AM389X_EmacPowerOn\r\n"));*/
    /*OALMSGS(OAL_ETHER&&OAL_FUNC, ( L"-AM389X_EmacPowerOn\r\n" ));*/
}

VOID AM389X_EmacPowerOff(VOID)
{
	/*
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"+AM389X_EmacPowerOff\r\n"));
    OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-AM389X_EmacPowerOff\r\n"));*/
}
//========================================================================
//!  \fn BOOL AM389X_EmacInit(BYTE *pbBaseAddress, DWORD dwMultiplier, USHORT MacAddr[3])
//!  \brief EMAC Initialization Routine.
//!
//!         This function initializes the EMAC for use in the boot-loader
//!         and KITL.
//!
//!  \param pbBaseAddress BYTE* Base address of the EMAC registers.
//!  \param dwMultiplier DWORD Offsets between the successive registers.
//!  \param MacAddr[3] USHORT Pointer to the array of 3 USHORTS to return MAC address.
//!  \return BOOL True if successful. False otherwise.
//========================================================================
BOOL AM389X_EmacInit (BYTE*   pbBaseAddress, DWORD   dwMultiplier, USHORT  MacAddr[3])
{
    BOOL    RetVal = FALSE;
    USHORT  Count = 0;
    UINT    clkdiv;
    UINT*   pu32RegPtr = NULL;
    UINT8*  pucMacAddr ;
    PEMAC_DESC pRXDesc;
	UINT32  uiRet, uiCtrl;

    OALMSGS(1/*OAL_ETHER&&OAL_FUNC*/, (
        L"+AM389X_EmacInit(0x%08x, 0x%08x, 0x%08x)\r\n", pbBaseAddress, 
        dwMultiplier, MacAddr ));
        
    /* Check if the base address passed is non-null. */
    if ( NULL == pbBaseAddress ){
        RetVal = FALSE;
        return (RetVal);
    } else {
        /* For OAL, we get pbBaseAddress virtual uncached address here */
        f_pEmacRegs  = (PEMAC_REGS)OALPAtoUA((UINT32)pbBaseAddress);
        f_pEmacCtlRegs = (PEMAC_CTRL_REGS)OALPAtoUA((UINT32)pbBaseAddress + EMAC_CTRL_OFFSET);
        f_pMdioRegs  = (PEMAC_MDIO_REGS)OALPAtoUA((UINT32)pbBaseAddress + EMAC_MDIO_OFFSET);
    }

    /* Assigning PLL1 register for PLL1 multiplier value */
//    pPLLRegs= (PPLLC_REGS)OALPAtoUA(PLLC1_BASE);
    pucMacAddr = (UINT8 *) MacAddr;

    /* Power on the EMAC subsystem */
    AM389X_EmacPowerOn();
    OALMSGS (OAL_WARN, (L"MAC addr is %02x:%02x:%02x:%02x:%02x:%02x.\r\n",
                   *(pucMacAddr + 0), *(pucMacAddr + 1), *(pucMacAddr + 2),
                   *(pucMacAddr + 3), *(pucMacAddr + 4), *(pucMacAddr + 5)));

    /*Issuing a reset to EMAC soft reset register and polling till we get 0. */
    f_pEmacRegs->m_Softreset = 1;
    while ( f_pEmacRegs->m_Softreset != 0 );

    /* Clear any pending interrupts */
    f_pEmacCtlRegs->c0_rx_en =0x0;
    f_pEmacCtlRegs->c0_tx_en =0x0;

    /* Clear MACCONTROL, RXCONTROL & TXCONTROL registers */
    f_pEmacRegs->m_Maccontrol = 0x0;
    f_pEmacRegs->m_Txcontrol  = 0x0;
    f_pEmacRegs->m_Rxcontrol  = 0x0;

    /* Initialise the 8 Rx/Tx Header descriptor pointer registers */
    pu32RegPtr  = (UINT *)&f_pEmacRegs->m_Tx0hdp;
    for (Count = 0; Count < EMAC_MAX_CHAN; Count++){
        *pu32RegPtr ++ = 0;
    }
    pu32RegPtr  = (UINT *)&f_pEmacRegs->m_Rx0hdp;
    for (Count = 0; Count < EMAC_MAX_CHAN; Count++){
        *pu32RegPtr ++ = 0;
    }
    /* Clear 36 Statics registers */
    /* The statics registers start from RXGODFRAMES and continues for the next
     * 36 DWORD locations.
     */
    pu32RegPtr  = (UINT *)&f_pEmacRegs->m_Rxgoodframes;
    for (Count=0; Count < EMAC_STATS_REGS; Count++){
        *pu32RegPtr ++ = 0;
    }

    /* Setup the local MAC address for all 8 Rx Channels */
    for (Count=0; Count < EMAC_MAX_CHAN; Count++){
        f_pEmacRegs->m_Macindex = Count;
        /* Filling MACADDRHI registers only for first channel */
        if (Count==0 ){
            f_pEmacRegs->m_Macaddrhi = ((*(pucMacAddr + 3) << 24)|(*(pucMacAddr + 2) << 16) |
                                        (*(pucMacAddr + 1) << 8) |(*(pucMacAddr + 0) << 0));
        }
        f_pEmacRegs->m_Macaddrlo = ((*(pucMacAddr + 5) << 8) |(*(pucMacAddr + 4) << 0))| (3 << 19);
    }

    /* clear the MAC address hash registers to 0 */
    f_pEmacRegs->m_Machash1 = 0;
    f_pEmacRegs->m_Machash2 = 0;

    /* Setup the local MAC address for 0th Transmit channel */
    f_pEmacRegs->m_Macindex = 0;
    f_pEmacRegs->m_Macsrcaddrhi = ((*(pucMacAddr + 3) << 24)|(*(pucMacAddr + 2) << 16) |
                                  (*(pucMacAddr + 1) << 8) |(*(pucMacAddr + 0) << 0));

    f_pEmacRegs->m_Macsrcaddrlo = ((*(pucMacAddr + 5) << 8) |(*(pucMacAddr + 4) << 0));

    /* Initialize the receive channel free buffer register including the
     * count, threshold, filter low priority frame threshold etc.
     * Currently, in boot-loader we don't bother about the flow-control
     * so skipping initialization of buffer control registers.*/

    /* clear the MAC address hash registers to 0 */
    f_pEmacRegs->m_Machash1 = 0;
    f_pEmacRegs->m_Machash2 = 0;

    /* Zero the receive buffer offset register */
    f_pEmacRegs->m_Rxbufferoffset = 0;

    /* Clear all the UniCast receive . This will effectively disalbe any packet
     * reception.*/
    f_pEmacRegs->m_Rxunicastclear = 0xFF;

    /* Setup receive multicast/broadcast/promiscous channel enable on channel 0.
     * We don't need to enable the multicast/promiscous modes. However, we do
     * need the broadcast receive capability.
     */
    f_pEmacRegs->m_Rxmbpenable = EMAC_RXMBPENABLE_RXBROADEN; 

    /* Setup MACCONTROL register with apt value */
    f_pEmacRegs->m_Maccontrol = EMAC_MACCONTROL_FULLDUPLEX_ENABLE;

    /* Clear all unused Tx and RX channel interrupts */
    f_pEmacRegs->m_Txintmaskclear = 0xFF;
    f_pEmacRegs->m_Rxintmaskclear = 0xFF;

    /* Enable the Rx and Tx channel interrupt mask registers. However, for boot-
     * loader, we don't need interrupts to be enabled. Thus, skip this step.
    */

    /* Prepare the buffers for Tx and Rx */
    f_pEmacRxDesc=(PEMAC_DESC)OALPAtoUA((UINT32)pbBaseAddress + EMAC_RAM_OFFSET + EMAC_RX_DESC_BASE);
    f_pEmacTxDesc=(PEMAC_DESC)OALPAtoUA((UINT32)pbBaseAddress + EMAC_RAM_OFFSET + EMAC_TX_DESC_BASE);

    /* Create the buffer descriptors for Rx-Ch0 */
    pRXDesc = f_pEmacRxDesc;
    f_pEmacRx_ActiveHead = f_pEmacRxDesc;
    for ( Count=0; Count < EMAC_MAX_RX_BUFFERS; Count++ ){
        pRXDesc->m_pNext =   (PEMAC_DESC)OALVAtoPA(pRXDesc + 1);
        pRXDesc->m_pBuffer = f_pucPhyRxBuffer +(Count*EMAC_MAX_PKT_BUFFER_SIZE);
                                          
        pRXDesc->m_BufOffLen = EMAC_MAX_ETHERNET_PKT_SIZE;
        pRXDesc->m_PktFlgLen = EMAC_DSC_FLAG_OWNER;
		++pRXDesc;
    }

    /* Set the last descriptor's "next" parameter to 0 to end the RX desc list */
    --pRXDesc;
    pRXDesc->m_pNext = NULL;
    f_pEmacRx_ActiveTail = pRXDesc;
    /* Since we are using only one TX descriptor setting it's members here only */ 
    f_pEmacTxDesc->m_pNext  = NULL;
    f_pEmacTxDesc->m_pBuffer= (UINT8*)(f_pucPhyRxBuffer + EMAC_RX_BUFS_SIZE);
    
    /* Adjust RX Length characteristics */
    f_pEmacRegs->m_Rxmaxlen = EMAC_RX_MAX_LEN;

    /* Initialize the MDIO */
    clkdiv = MHZ(250); // (DVEVM_OSC_FREQ*(pPLLRegs->m_Pllm + 1)/SYSCLK5);
    clkdiv = (clkdiv/(EMAC_MDIO_CLOCK_FREQ)) - 1;
    f_pMdioRegs->m_Control = ((clkdiv & 0xFF) |(MDIO_CONTROL_ENABLE) |(MDIO_CONTROL_FAULT));

    /* Enable Tx & Rx */

    /* Determining which  PHY address has a link */
    AM389X_EmacGetLink(f_Link);        

	OALMSGS(1,(L"f_PhyAddr 0x%08X\r\n", f_PhyAddr));

	// enable output of 1000Base-T Phy TX_CLK pin  
	uiRet = AM389X_EmacMdioRead(f_PhyAddr, MII_PHY_CONFIG_REG);
	uiRet |= PHY_TX_CLK_EN;
	AM389X_EmacMdioWrite(f_PhyAddr, MII_PHY_CONFIG_REG,uiRet);
	uiRet = AM389X_EmacMdioRead(f_PhyAddr, MII_PHY_CONFIG_REG);
	OALMSGS(1,(L"MII_PHY_CONFIG_REG 0x%08X\r\n", uiRet));

	// update MACCONTROL register	
	uiRet = AM389X_EmacMdioRead(1, MII_PHY_STATUS_REG);
	OALMSGS(1,(L"MII_PHY_STATUS_REG 0x%08X\r\n", uiRet));

	uiCtrl = f_pEmacRegs->m_Maccontrol;
	OALMSGS(1,(L"m_Maccontrol 0x%08X\r\n", uiCtrl));
	if (uiRet & PHY_DUPLEX_MASK)
		uiCtrl |= EMAC_MACCONTROL_FULLDUPLEX_ENABLE;
	else
		uiCtrl &= ~EMAC_MACCONTROL_FULLDUPLEX_ENABLE;

	if ((uiRet & PHY_SPEED_MASK) == PHY_SPEED_1000BASET)
		uiCtrl |= EMAC_MACCONTROL_GIG_ENABLE | EMAC_MACCONTROL_GIGFORCE;
	else
		uiCtrl &= ~(EMAC_MACCONTROL_GIG_ENABLE | EMAC_MACCONTROL_GIGFORCE);
		
	f_pEmacRegs->m_Maccontrol = uiCtrl;

    f_pEmacRegs->m_Rxunicastset = 0x1;
    f_pEmacRegs->m_Txcontrol = 0x1;
    f_pEmacRegs->m_Rxcontrol = 0x1;
    f_pEmacRegs->m_Maccontrol |= (EMAC_MACCONTROL_MIIEN_ENABLE);

	OALMSGS(1,(L"m_Maccontrol 0x%08X\r\n", f_pEmacRegs->m_Maccontrol));
         
    /* Start receive process */
    f_pEmacRegs->m_Rx0hdp = (UINT)OALVAtoPA(f_pEmacRxDesc);
    RetVal =TRUE;
    OALMSGS(1 /*OAL_ETHER&&OAL_FUNC*/, (L"-AM389X_EmacInit(rc = %d)\r\n", RetVal));
    return(RetVal);
}

//========================================================================
VOID AddToTailEnd( PEMAC_DESC pInDesc )
// Adds individual descriptor to list.
{
    PEMAC_DESC pRXTail;
    UINT8* pBuffer = pInDesc->m_pBuffer; // this member is currently containing
                                         // physical address itself.
    /* Clear the buffer descriptor */
    memset ((char *) pInDesc, 0x0, sizeof (EMAC_DESC));
    pRXTail = f_pEmacRx_ActiveTail;

    /* Recycle RX descriptor */
    pInDesc->m_BufOffLen    = EMAC_MAX_ETHERNET_PKT_SIZE;
    pInDesc->m_PktFlgLen    = EMAC_DSC_FLAG_OWNER;
    pInDesc->m_pNext = NULL;
    pInDesc->m_pBuffer = pBuffer ; // Updating with physical address
    
    /* Re-use the local variable to keep the physical addr of Desc handy
     * we don't want to waste too much time calculating this after performing
     * the check for EOQ bit.
     */
    pBuffer = (PUCHAR) OALVAtoPA(pInDesc);

    /* If the EOQ bit in the existing tail is set, then we need to update
     * the Rx[0]HDP register. Else we can just add to the tail.
     */
    if ( 0 != (pRXTail->m_PktFlgLen & EMAC_DSC_FLAG_EOQ ))
        f_pEmacRegs->m_Rx0hdp = (UINT)pBuffer;
    
    pRXTail->m_pNext= (PEMAC_DESC)pBuffer;
    /* Reassign the global tail to fresh packet added */
    f_pEmacRx_ActiveTail= pInDesc;

    return;
}

//========================================================================
VOID DiscardPkt( PEMAC_DESC pInDesc )
// Disacrds entire unprocessed packet.
{
    PEMAC_DESC pCurDesc;
    PEMAC_DESC pNextDesc;

    /* Return if Null packet */
    if ( pInDesc == NULL )
        return;

	/* Discard the descriptors till it is EOP */
    /* Initially all point to same */
    pCurDesc = pNextDesc = pInDesc;

	do {/* till we get EOP packet */
        /* Point to next descriptor */
        pCurDesc = pNextDesc;

        /* Point to next current descriptor */
        pNextDesc=OALPAtoUA((UINT32)pCurDesc->m_pNext);

        /* Add it to end of present tail */
        AddToTailEnd(pCurDesc);

        /* Ack received packet descriptor */
        f_pEmacRegs->m_Rx0cp = (UINT32) OALVAtoPA(pCurDesc);

        /* If the next packet has the SOP bit set, then break. Perhaps the
        * next packet is a valid frame
        */
        
        /* 
         * Also check if the pNextDesc is still being owned by the EMAC. 
         * We should  be checking for SOP flag only if it is not owned by
         * EMAC. If the EMAC is still owning the buffer, then we must also 
         * break.
         */
        if (( pNextDesc->m_PktFlgLen & EMAC_DSC_FLAG_SOP ) ||
            ( pNextDesc->m_PktFlgLen & EMAC_DSC_FLAG_OWNER))
            break;

	} while ( 0 != (pCurDesc->m_PktFlgLen & EMAC_DSC_FLAG_EOP) );

    /* Update RxHead to point to the received descriptor before returning */
    f_pEmacRx_ActiveHead = pNextDesc;

    return;
}



VOID OEMWriteDebugByte(UINT8 ch);

//========================================================================
UINT16 AM389X_EmacGetFrame( BYTE* pbData, UINT16* pwLength )
{
    BOOL PktRxComplete = FALSE ;
    BOOL PktRxStarted  = FALSE ;
    BOOL PktDiscard    = TRUE;
    UINT16 BufLen;
    UINT16 PktLen;
    UINT16 FrmLen = 0;
    UINT16 RetVal = 0;
    UINT32 Status;
    PUCHAR pRxBuffer;

//	BYTE* pbData_tmp = pbData;

    PEMAC_DESC pCurDesc;
    PEMAC_DESC pNextDesc;

    if(TRUE == f_InterruptMode) OEMWriteDebugByte('A'); 

    if ( (f_pEmacRegs->m_Rxintstatraw & (BIT0)) == 0x00 ){
		if(TRUE == f_InterruptMode) OEMWriteDebugByte('E'); 
        *pwLength = 0;
		goto exit_here;
//        return (FrmLen);
    }
    
    if(TRUE == f_InterruptMode){
		f_pEmacCtlRegs->c0_rx_en =0x0;
	    f_pEmacCtlRegs->c0_tx_en =0x0;
		/* Clearing channel0 RX channel interrupts */
		f_pEmacRegs->m_Rxintmaskclear = 0x1;
    }
    /* While the bCompletePktRx is FALSE keep looping the following algo */
    while ( FALSE == PktRxComplete ){
        
        pCurDesc=f_pEmacRx_ActiveHead;   /*Point to current head */
        while ( 0 == (f_pEmacRegs->m_Rxintstatraw & (BIT0)) ); // is there any packet reception completed

        /* Take the copy of the next pointer in the buffer for future ref */
        Status    = pCurDesc->m_PktFlgLen;
        pNextDesc = (PEMAC_DESC)OALPAtoUA((DWORD)pCurDesc->m_pNext);
        pRxBuffer = (PUCHAR)OALPAtoUA((DWORD)pCurDesc->m_pBuffer);

//OALMSG(1,(L"RX pCurDesc-%08X; Status-%08X; pNextDesc-%08X; pRxBuffer-%08X\r\n",
//	     pCurDesc, Status, pNextDesc, pRxBuffer));        

		/* If the head descriptor is pointing to the buffer having the OWNER
         * flag indicating the EMAC as owner, then we have mysteriously come
         * here. So get out of here without disturbing anything. Not sure if
         * we should be causing the teardown for this condition
         */
        if ( 0 != (EMAC_DSC_FLAG_OWNER & Status )){
            /* we are ok to break here as further down we are re-assigning
             * the head pointer back to the pCurDesc (as we have not processed
             * any packet, its good to do it). However, in other cases, we
             * are freeing up the buffers and we should be cautious about
             * re-assigning the head pointer
             */
            OALMSGS (1/*OAL_WARN*/,(L"Still held by EMAC. 0x%08X\r\n",pCurDesc));
            break;
        }

        /* Process  the start of packet buffer */
        if ( 0 != (Status & EMAC_DSC_FLAG_SOP )){
            /* Process if a error in packet */
            if ( 0 != (Status & EMAC_DSC_RX_ERROR_FRAME )){
                OALMSGS (1/*OAL_WARN*/,(L"Error in packet. Status = 0x%x\r\n", Status));
            } else {
                PktLen=Status & 0xFFFF;
                /* Check packet size */
                if ( (PktLen > *pwLength) || (PktLen < EMAC_MIN_ETHERNET_PKT_SIZE) ) {
                    OALMSGS(1/*OAL_WARN*/,(L"Packet sizes are out of min&max boundary  %u %u\r\n",
                                 PktLen,*pwLength));
                } else {
                    /* Only now we are good to receive the packet */
                    PktRxStarted = TRUE;
                    PktDiscard  = FALSE;
                }
            }
        } else if ( FALSE == PktRxStarted ){ /* Process all non-SOP packets */
            OALMSGS(OAL_WARN,(L"Packets received without SOP  \r\n"));
            PktDiscard = TRUE;
        }
        /* Check discard  flag & take necessary steps */
        if ( TRUE == PktDiscard ){
            OALMSGS(1/*OAL_WARN*/, (L"Discarding the packet.\r\n"));
            DiscardPkt (pCurDesc);
            return (FrmLen);
        } else {
            /* Process the descriptor */
            BufLen = pCurDesc->m_BufOffLen & 0xFFFF;
            if((FrmLen + BufLen) > *pwLength){
                OALMSGS(1/*OAL_WARN*/,(L"Buffer length exceeding input length.\r\n")); 
                //  *pwLength=0;
                //  return 0;
            }    
            /* copy to local buffer */
            memcpy (pbData, pRxBuffer, BufLen);
            /* Process buffer and length */
            pbData += BufLen;
            FrmLen += BufLen;
		}
        /* Processs if EOP packet */
        if ( 0 != (Status & EMAC_DSC_FLAG_EOP )){
            PktRxComplete = TRUE;
            *pwLength  = FrmLen;
            RetVal     = FrmLen;
        }
        /* Ack received packet descriptor */
        f_pEmacRegs->m_Rx0cp = (UINT32)OALVAtoPA(pCurDesc);
        /* Add processed packet to tail */
        AddToTailEnd (pCurDesc);
        f_pEmacRx_ActiveHead = pNextDesc;

        /* we are likely to reach here if the EMAC ran out of buffer and we have
           actually submitted the second list */
        if ((NULL == f_pEmacRx_ActiveHead) ||(f_pEmacRx_ActiveHead == OALPAtoUA(0x0))){
            f_pEmacRx_ActiveHead = f_pEmacRxDesc;
        }
    } // while

exit_here:
    if(TRUE == f_InterruptMode){
        /* Enabling global interrupt enable */
		f_pEmacRegs->m_Maceoiector = 0x1;	
	    f_pEmacCtlRegs->c0_rx_en =0x01;
//		f_pEmacRegs->m_Rxintmaskset = 0x1;
		
//	    f_pEmacCtlRegs->c0_tx_en =0xff;
	}
        
    /* End of bCompletePktRx */
    /*OALMSGS(OAL_ETHER&&OAL_FUNC, (L"-AM389X_EmacGetFrame(length = %d)\r\n", FrmLen));*/
//    OALMSGS(1,(L"<<<<RX Exit %d\r\n", (DWORD)*pwLength));
//	MyDumpFrame(pbData_tmp, (DWORD)*pwLength );

	return (FrmLen);
}


//========================================================================
UINT16 AM389X_EmacSendFrame(BYTE* pbData, DWORD dwLength )
// pbData BYTE* Buffer of frame data to send.
// dwLength DWORD Length of buffer to send.
// return UINT16 Number of bytes sent.
{
    UINT32 RetVal=(UINT32)-1;
    UINT32 Address;
    PUCHAR puTxBuffer = OALPAtoUA((DWORD)f_pEmacTxDesc->m_pBuffer);

//	MyDumpFrame(pbData, dwLength);

    if (dwLength > EMAC_RX_MAX_LEN) 
        return (UINT16)(RetVal);

    if ( 0 == AM389X_EmacGetLink(f_Link) ) {
        OALMSGS (1/*OAL_ERROR*/,(L"AM389X_EmacSendFrame: Link Down \r\n"));
        return (UINT16)(RetVal);
    }

    /* Filling the descriptor values */
    memset((void*)puTxBuffer,0 ,EMAC_MAX_ETHERNET_PKT_SIZE);    
    memcpy((void*)puTxBuffer,pbData,dwLength);    

	if ( dwLength < EMAC_MIN_ETHERNET_PKT_SIZE )
        dwLength = EMAC_MIN_ETHERNET_PKT_SIZE;

    f_pEmacTxDesc->m_BufOffLen= (dwLength & 0xFFFF);
    f_pEmacTxDesc->m_PktFlgLen=(EMAC_DSC_FLAG_SOP   | EMAC_DSC_FLAG_EOP |
								EMAC_DSC_FLAG_OWNER | (dwLength & 0xFFFF));

    /* Update the TxHDP to above descriptor to transmit */
    f_pEmacRegs->m_Tx0hdp = (UINT)OALVAtoPA(f_pEmacTxDesc);

    /* Polling transmit has happened by unmasked register*/
    while ( 0 == (f_pEmacRegs->m_Txintstatraw & (0x1)) );
    
    for (;;){ /* Check if any error has occured or not */
        /* Check  TX0CP register  to get status */
        Address = f_pEmacRegs->m_Tx0cp;
        if ( Address != (UINT32)OALVAtoPA(f_pEmacTxDesc)){
            OALMSGS (1/*OAL_WARN*/,(L"AM389X_EmacSendFrame:Transmit failed\r\n"));    
            break;
		}
        /* Check if teardowm has happened */
        if ( f_pEmacTxDesc->m_PktFlgLen & EMAC_DSC_FLAG_TDOWNCMPLT ){
            OALMSGS (1/*OAL_WARN*/,(L"AM389X_EmacSendFrame:Teardown happened\r\n"));    
            break;
        }

		/* TX has been completed successfully */
        RetVal= 0;
        break;
	}
    return (UINT16)(RetVal);
}

//========================================================================
void AM389X_EmacEnableInts(void)
// Enable interrupts on EMAC subsystem. TODO !!!!!!!!!!!!!!
{

OEMWriteDebugByte('W'); 
	f_pEmacRegs->m_Maceoiector = 0x1;	
//	f_pEmacRegs->m_Maceoiector = 0x2;	
//	f_pEmacRegs->m_Maceoiector = 0x3;	
    f_pEmacCtlRegs->c0_rx_en = 0x01;
//    f_pEmacCtlRegs->c0_tx_en =0xff;

    /* Setting channel0   RX channel interrupts */
    f_pEmacRegs->m_Rxintmaskset = 0x1;
    
    f_InterruptMode = TRUE;
}

//========================================================================
void AM389X_EmacDisableInts(void)
// brief Disable interrupts on EMAC subsystem. TODO !!!!!!!!!!!!!! 
{
	OEMWriteDebugByte('D'); 

OALMSGS (OAL_WARN,(L"AM389X_EmacDisableInts\r\n"));

		 /* Clearing global interrupt enable */
    f_pEmacCtlRegs->c0_rx_en =0x0;
//    f_pEmacCtlRegs->c0_tx_en =0x0;

	/* Clearing channel0 RX channel interrupts */
    f_pEmacRegs->m_Rxintmaskclear = 0x1;
    f_InterruptMode = FALSE;
}

//========================================================================
VOID AM389X_EmacCurrentPacketFilter(UINT32 filter)
// brief Called to set hardware  filter.
{
    UINT32 RxMBPRegVal = f_pEmacRegs->m_Rxmbpenable; 

    if((filter & PACKET_TYPE_DIRECTED) || (filter & PACKET_TYPE_BROADCAST)){
        /*  EDBG always receive directed and broadcast as a minimum.
         *  So we always have unicast and broadcast always enabled.
         *  No need to be taken here. 
         */
    }                
    /* We are taking care multicast enable and promiscuos mode enable 
     * filter flags as of now.
	 */  
    if ((filter & PACKET_TYPE_ALL_MULTICAST) || (filter & PACKET_TYPE_MULTICAST)){
        RxMBPRegVal |= (1 << 5);
    }

    if (filter & PACKET_TYPE_PROMISCUOUS){
        RxMBPRegVal |= EMAC_RXMBPENABLE_RXCAFEN_ENABLE;
    }
    f_pEmacRegs->m_Rxmbpenable = RxMBPRegVal;       
}    

//========================================================================
USHORT HashAddress(UCHAR* pAddress)
// Implements a hashing function for multicast addresses.
// UCHAR* pAddress Six bytes MAC address
// USHORT Six bit hash value.
{
	UINT OutIndex;
	UINT InIndex;
	UINT Result=0;
	USHORT HashFunVal;
	UCHAR BitPos;
	UCHAR MACAddrIndex;
   
    for(OutIndex = 0; OutIndex < 6; OutIndex++) {
        HashFunVal =0; /* After each of 6 bits calculation value should be reset to low */
        for(InIndex = 0; InIndex < 8; InIndex++) {
            BitPos = (UCHAR)(OutIndex + 6*InIndex);
            MACAddrIndex = 5 - BitPos/8 ;
            HashFunVal ^= ( pAddress[MACAddrIndex] >> (BitPos % 8)) & 0x1 ;
        }
        Result |= HashFunVal << OutIndex;
    }
	return (USHORT)(Result & 0x3F);
}

//========================================================================
BOOL AM389X_EmacMulticastList(UINT8 *pAddresses, UINT32 count )
{
    BOOL rc=TRUE;
    UINT32 Index;
    UINT32 HashValue=0;
    
    for (Index =0; Index < count; Index++){
        HashValue = HashAddress(pAddresses + Index*6);
        if (HashValue < 32 ){
            f_pEmacRegs->m_Machash1 = (0x1 << HashValue);
        } else if ((HashValue >= 32 ) && (HashValue < 64 )) {    
            f_pEmacRegs->m_Machash2 = (0x1 << (HashValue-32));
        } else {
            /* Should never happen only means hashing function 
             * is wrongly implemented, anyways we will return false */
            rc = FALSE;       
        }
    }   
    return (rc);
}// 1087
