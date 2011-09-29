//-----------------------------------------------------------------------------
//! \addtogroup	OAL
//! @{
//!
//  All rights reserved ADENEO 2007
//!
//-----------------------------------------------------------------------------
//! \file		OAL\intr.c
//!
//! \brief		Implements the BSP specific interrupts management
//!
//! \if subversion
//!   $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9261EK_v170_rc4/PLATFORM/AT91SAM9261EK/SRC/KERNEL/OAL/intr.c $
//!   $Author: pblanchard $
//!   $Revision: 1283 $
//!   $Date: 2007-08-10 11:06:42 +0200 (ven., 10 août 2007) $
//! \endif
//-----------------------------------------------------------------------------


//! \addtogroup	INTR
//! @{

#include <windows.h>
#include <oal.h>
#include <oal_intr.h>
#include <nkintr.h>
#include <intr.h>
#include "AT91SAM926x.h"
#include "lib_AT91SAM926x.h"
#include "AT91SAM926x_oal_intr.h"
#include "AT91SAM9261EK.h"
#include "pio_intr.h"
#include "drv_glob.h"

#include "em9161_isa.h"

//
// CS&ZHL MAY-18-2009: use ISA_IRQ1(PA20) as shared interrupt source
//
#define	 EM9161_SHARED_IRQ_ID			(LOGINTR_BASE_PIOA + 20)				// -> PA20

//
// CS&ZHL MAY-18-2009: globe variances for shared interrupt processing
//
volatile DWORD	g_dwNumOfIrq;
volatile UCHAR  g_uSIrqValue;
volatile DWORD	g_dwIrqInfoAddr;



//-----------------------------------------------------------------------------
//! \fn			  BOOL BSPIntrInit()
//!
//! \brief		This function allows the BSP to initialize a secondary interrupt controller.
//!
//! \note     BSPIntrInit is called by OALIntrInit if your implementation uses hardware platform 
//!     callbacks. OALIntrInit initializes the interrupt hardware and is usually called during 
//!     the system initialization process.
//!
//!
//!
//!	\return		TRUE indicates success
//! \return   FALSE indicates failure
//-----------------------------------------------------------------------------
BOOL BSPIntrInit()
{
    OALMSG(OAL_INTR&&OAL_FUNC, (L"+BSPIntrInit\r\n"));

#ifdef	EM9161_SHARED_IRQ_ENABLE
	g_dwNumOfIrq = 0;
	g_uSIrqValue = 0;
	g_dwIrqInfoAddr = 0;
#endif	//EM9161_SHARED_IRQ_ENABLE

    OALMSG(OAL_INTR&&OAL_FUNC, (L"-BSPIntrInit\r\n"));

    return TRUE;
}

//-----------------------------------------------------------------------------
//! \fn			  BOOL BSPIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
//!
//! \brief		This functions returns a physical interrupt number associated with a device being at the given location
//!
//! \note     This is used in our case to retrieve the interrupt the DM9000 controller
//!
//!
//!
//!	\return		TRUE indicates success
//! \return   FALSE indicates failure
//-----------------------------------------------------------------------------
BOOL BSPIntrRequestIrqs(DEVICE_LOCATION *pDevLoc, UINT32 *pCount, UINT32 *pIrqs)
{
    BOOL rc = FALSE;

    OALMSG(OAL_INTR&&OAL_FUNC, (
        L"+BSPIntrRequestIrq(0x%08x->%d/%d/0x%08x/%d, 0x%08x, 0x%08x)\r\n", 
        pDevLoc, pDevLoc->IfcType, pDevLoc->BusNumber, pDevLoc->LogicalLoc,
        pDevLoc->Pin, pCount, pIrqs
    ));
	DEBUGMSG(1,(L"+BSPIntrRequestIrq\r\n"));


   	if (pDevLoc->LogicalLoc == AT91C_BASE_DM9000) 
	{
		// Request IRQ for DM9000
		OALMSGS(TRUE, (L"IRQ for DM9000\r\n"));
		
		if (pIrqs)
		{
			// INT pin is on PIO C11
			rc = TRUE;
			*pIrqs = IRQ_DM9000;
		}
	}

	DEBUGMSG(1,(L"-BSPIntrRequestIrq\r\n"));
    OALMSG(OAL_INTR&&OAL_FUNC, (L"-BSPIntrRequestIrq(rc = %d)\r\n", rc));
    return rc;
}

//-----------------------------------------------------------------------------
//! \fn			  UINT32 BSPIntrEnableIrq(UINT32 irq)
//!
//! \brief		This function is called from OALIntrEnableIrq to give a chance to 
//!      the BSP to enable the interrupt at the board level.
//!
//! \note     It can also do something else. (like enabling cascading IT lines)
//!
//! \param		irq Specifies the IRQ to be enabled.
//!
//!	\return		Returns the IRQ that must be enabled on the primary interrupt controller
//! \return   IRQ_UNDEFINED if no other IRQ needs to be enabled.
//-----------------------------------------------------------------------------
UINT32 BSPIntrEnableIrq(UINT32 irq)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrEnableIrq(%d)\r\n", irq));

#ifdef	EM9161_SHARED_IRQ_ENABLE
	{
		if((irq >= EM9161_ID_EX28) && (irq <= EM9161_ID_EX2B))
		{
			irq = EM9161_SHARED_IRQ_ID;
		}
	}
#endif	//EM9161_SHARED_IRQ_ENABLE

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrEnableIrq(irq = %d)\r\n", irq));
    return irq;
}

//-----------------------------------------------------------------------------
//! \fn			  UINT32 BSPIntrDisableIrq(UINT32 irq)
//!
//! \brief		This function is called from OALIntrDisableIrq to give a chance to 
//!      the BSP to enable the interrupt at the board level.
//!
//! \note     It can also do something else. (like enabling cascading IT lines)
//!
//! \param		irq Specifies the IRQ to be disabled.
//!
//!	\return		Returns the IRQ that must be disabled on the primary interrupt controller
//-----------------------------------------------------------------------------
UINT32 BSPIntrDisableIrq(UINT32 irq)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDisableIrq(%d)\r\n", irq));

#ifdef	EM9161_SHARED_IRQ_ENABLE
	{
		if((irq >= EM9161_ID_EX28) && (irq <= EM9161_ID_EX2B))
		{
			irq = EM9161_SHARED_IRQ_ID;
		}
	}
#endif	//EM9161_SHARED_IRQ_ENABLE

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDisableIrq(irq = %d\r\n", irq));
    return irq;
}

//-----------------------------------------------------------------------------
//! \fn			  UINT32 BSPIntrDoneIrq(UINT32 irq)
//!
//! \brief		 This function is called from OALIntrEnableIrq to give a chance to 
//!      the BSP to finish the interrupt at the board level.
//!
//! \param		irq Specifies the IRQ to be re-enabled.
//!
//!	\return		Returns the IRQ that must be scanned on the primary interrupt controller
//-----------------------------------------------------------------------------
UINT32 BSPIntrDoneIrq(UINT32 irq)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrDoneIrq(%d)\r\n", irq));

#ifdef	EM9161_SHARED_IRQ_ENABLE
	{
		if((irq >= EM9161_ID_EX28) && (irq <= EM9161_ID_EX2B))
		{
			irq = EM9161_SHARED_IRQ_ID;
			if(g_uSIrqValue == (EM9XXX_SHARE_IRQ_ENABLE | EM9XXX_SHARE_IRQ_ENTER_IST))
			{
				//
				// CS&ZHL MAY-18-2009: write 0x0a to SCR to exit IST state
				//
				g_uSIrqValue = EM9XXX_SHARE_IRQ_ENABLE | EM9XXX_SHARE_IRQ_EXIT_IST;	
				*((volatile UCHAR*)g_dwIrqInfoAddr) = g_uSIrqValue;
			}
		}
	}
#endif	//EM9161_SHARED_IRQ_ENABLE

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrDoneIrq(irq = %d)\r\n", irq));
    return irq;
}


//-----------------------------------------------------------------------------
//! \fn			  UINT32 BSPIntrActiveIrq(UINT32 irq)
//!
//! \brief		 This function is called from OALIntrEnableIrq to give a chance to 
//!      translate IRQ in case for example of secondary interrupt controller on the board
//!
//! \param		irq The IRQ to be translated.
//!
//!	\return		Returns the IRQ that must be translated on the primary interrupt controller.
//-----------------------------------------------------------------------------
UINT32 BSPIntrActiveIrq(UINT32 irq)
{
    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"+BSPIntrActiveIrq(%d)\r\n", irq));

#ifdef	EM9161_SHARED_IRQ_ENABLE
	{
		UINT8	IrqStatus;

		if((irq == EM9161_SHARED_IRQ_ID) && (g_dwNumOfIrq != 0))
		{
			IrqStatus = *((volatile UCHAR*)g_dwIrqInfoAddr);
			irq = EM9161_ID_EX28 + (UINT32)(IrqStatus & 0x03);

			//
			// CS&ZHL MAY-18-2009: write 0x0d to SCR to enter IST state
			//
			g_uSIrqValue = EM9XXX_SHARE_IRQ_ENABLE | EM9XXX_SHARE_IRQ_ENTER_IST;	//=>0x0d
			*((volatile UCHAR*)g_dwIrqInfoAddr) = g_uSIrqValue;
		}
	}
#endif	//EM9161_SHARED_IRQ_ENABLE

    OALMSG(OAL_INTR&&OAL_VERBOSE, (L"-BSPIntrActiveIrq(%d)\r\n", irq));
    return irq;
}


//! @} end of subgroup INTR

//! @} end of group OAL

//-----------------------------------------------------------------------------
// End of $URL: http://centaure/svn/interne-ce_bsp_atmel/TAGS/TAGS50/SAM9261EK_v170_rc4/PLATFORM/AT91SAM9261EK/SRC/KERNEL/OAL/intr.c $
//-----------------------------------------------------------------------------
//
