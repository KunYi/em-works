//-----------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Use of this source code is subject to the terms of the Microsoft end-user
//  license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
//  If you did not accept the terms of the EULA, you are not authorized to use
//  this source code. For a copy of the EULA, please see the LICENSE.RTF on
//  your install media.
//
//-----------------------------------------------------------------------------
//
//  Copyright (C) 2007-2009, Freescale Semiconductor, Inc. All Rights Reserved.
//  THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
//  AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
//  File:  flash.c
//
//  Contains flash functions used by boot loader.
//
//-----------------------------------------------------------------------------

#include "bsp.h"
#include "loader.h"

//-----------------------------------------------------------------------------
// External Functions
BOOL NORWrite(DWORD dwStartAddr, DWORD dwLength);
BOOL NORLoadBootCFG(BYTE * pBootCfg, DWORD cbBootCfgSize);
BOOL NORStoreBootCFG(BYTE * pBootCfg, DWORD cbBootCfgSize);
BOOL NANDWriteXldr(DWORD dwStartAddr, DWORD dwLength);
BOOL NANDMakeMBR(void);											// CS&ZHL APR-1-2012: supporting BinFS
BOOL NANDWriteSB(DWORD dwStartAddr, DWORD dwLength);
BOOL NANDWriteIPL(DWORD dwStartAddr, DWORD dwLength);
BOOL NANDStartWriteBinDIO(DWORD dwStartAddr, DWORD dwLength);
BOOL NANDContinueWriteBinDIO(DWORD dwAddress, BYTE *pbData, DWORD dwSize);
BOOL NANDFinishWriteBinDIO();
BOOL NANDWriteNK(DWORD dwStartAddr, DWORD dwLength);
BOOL NANDLoadBootCFG(BYTE * pBootCfg, DWORD cbBootCfgSize);
BOOL NANDStoreBootCFG(BYTE * pBootCfg, DWORD cbBootCfgSize);
BOOL SDHCLoadBootCFG(BYTE * pBootCfg, DWORD cbBootCfgSize);
BOOL SDHCStoreBootCFG(BYTE * pBootCfg, DWORD cbBootCfgSize);
BOOL SDHCWriteXldr(DWORD dwStartAddr, DWORD dwLength);
BOOL MX28_SDHCWriteSB(DWORD dwStartAddr, DWORD dwLength);
BOOL SDHCWriteNK(DWORD dwStartAddr, DWORD dwLength);

//
// CS&ZHL MAY-23-2011: add generic NandFlash routines for NK
//
extern BOOL EBOOT_ReadFlash(DWORD dwRAMAddressDst, DWORD dwNANDAddressSrc, DWORD dwLen);
extern BOOL EBOOT_WriteFlash(DWORD dwRAMAddressSrc, DWORD dwNANDAddressDst, DWORD dwLen);
extern BOOL EBOOT_EraseFlash(DWORD dwNANDAddress, DWORD dwLen);

//-----------------------------------------------------------------------------
// External Variables
extern IMAGE_TYPE g_ImageType;
extern IMAGE_MEMORY g_ImageMemory;
extern BOOL g_bNandBootloader;
extern BOOL g_bSDHCBootloader;
extern BOOT_BINDIO_CONTEXT g_BinDIO;

//
// CS&ZHL MAY-19-2011: variables for BinFS
//
extern DWORD g_dwTotalBinNum;			// CS&ZHL MAY-21-2011: get from OEMMultiBINNotify()
extern DWORD g_dwCurrentBinNum;
extern DWORD g_dwBinAddress[];		// CS&ZHL MAY-21-2011: Flash Physical Addresses
extern DWORD g_dwBinLength[];			// Now we support 16 bin files for MultiBIN
//
// CS&ZHL MAY-23-2011: save dwImageStart and dwImageLength into BootCFG
//
extern BOOT_CFG g_BootCFG;

//-----------------------------------------------------------------------------
// Defines


//-----------------------------------------------------------------------------
// Types


//-----------------------------------------------------------------------------
// Global Variables


//-----------------------------------------------------------------------------
// Local Variables


//-----------------------------------------------------------------------------
//
//  Function:  OEMIsFlashAddr
//
//  This function determines whether the address provided lies in a platform's
//  flash memory or RAM address range.
//
//  Parameters:
//      dwAddr
//          [in] Address of a BIN file record. This is the address that is
//          checked to see whether it lies in flash or RAM.
//
//  Returns:
//      Returns TRUE if an address is a flash address, and FALSE if not.
//-----------------------------------------------------------------------------
BOOL OEMIsFlashAddr (DWORD dwAddr)
{
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwAddr);

    // OEMVerifyMemory will be called first to determine the destination of the
    // the image.  Use g_ImageMemory set by OEMVerifyMemory to determine if this
    // is a NOR/NAND/SD flash address.

    if ((g_ImageMemory == IMAGE_MEMORY_NOR) || (g_ImageMemory == IMAGE_MEMORY_NAND)
        || (g_ImageMemory == IMAGE_MEMORY_SD))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//-----------------------------------------------------------------------------
//
//  Function:  OEMMapMemAddr
//
//  This function remaps a flash-resident address to a unique RAM-based address
//  so that flash memory OS images can be temporarily cached in RAM while the
//  download occurs.
//
//  Parameters:
//      dwImageStart
//          [in] Starting address of OS image.
//
//      dwAddr
//          [in] Address of a BIN record. If this address lies in a
//          platform's flash memory address space, typically the offset from
//          dwImageStart is computed and added to a RAM-based file cache area.
//
//  Returns:
//      Address from which the BIN record should be copied to provide file
//      caching before and during the flash update process.
//-----------------------------------------------------------------------------
LPBYTE OEMMapMemAddr (DWORD dwImageStart, DWORD dwAddr)
{
    if (OEMIsFlashAddr(dwAddr))
    {
        if (g_ImageType == IMAGE_TYPE_BINDIO && g_ImageMemory == IMAGE_MEMORY_NAND)
        {
            // The record offset is needed in order to know where the image
            // is required to be flashed.  It's the offset of this one recored
            // from the image start.
            //
            // Note: The record can be a non contiguous image.
            //
            // The dwImageStart would always be the pInfo->Region[0].dwRegionStart
            // we saw in OEMMultiBINNotify().
            g_BinDIO.recordOffset = dwAddr - dwImageStart;

            // Always cache data to the same place since DIO is flashed
            // immediately on download and thus the buffer can be reused.
            dwAddr = BOOT_FLASHBLOCK_CACHE_START;
        }
        else
        {
            // The image being downloaded is a flash image - temporarily
            // cache the image in RAM until it's completely downloaded.
            //
            // dwAddr -= dwImageStart;
			//
			// CS&ZHL MAR-8-2012: if the image is destined for NandFlash, always substract "nand_start_addr" 
			//                    to support multiple image files (BinFS -> 3 image files)
			//
			if(dwImageStart >= IMAGE_BOOT_NKIMAGE_NAND_CA_START)
			{
        	    dwAddr -= IMAGE_BOOT_NKIMAGE_NAND_CA_START;
			}
			else
			{
        	    dwAddr -= dwImageStart;
			}
            dwAddr += BOOT_FLASHBLOCK_CACHE_START;
        }
    }

    return((LPBYTE) (dwAddr));
}


//-----------------------------------------------------------------------------
//
//  Function:  OEMStartEraseFlash
//
//  This function initiates the flash memory erasing process.
//
//  Parameters:
//      dwStartAddr
//          [in] Address in flash memory from which to start erasing.
//
//      dwLength
//          [in] Number of bytes of flash memory to be erased.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//-----------------------------------------------------------------------------
BOOL OEMStartEraseFlash (DWORD dwStartAddr, DWORD dwLength)
{
    BOOL rc = TRUE;

    if (g_ImageType == IMAGE_TYPE_BINDIO && g_ImageMemory == IMAGE_MEMORY_NAND)
    {
        rc = NANDStartWriteBinDIO(dwStartAddr, dwLength);
    }

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  OEMContinueEraseFlash
//
//  This function provides a means to continue erasing flash memory blocks
//  while a download is taking place.
//
//  Parameters:
//      None.
//
//  Returns:
//      None.
//-----------------------------------------------------------------------------
void OEMContinueEraseFlash(void)
{
    if (g_ImageType == IMAGE_TYPE_BINDIO && g_ImageMemory == IMAGE_MEMORY_NAND)
    {
        NANDContinueWriteBinDIO(g_BinDIO.recordOffset,
                                g_BinDIO.pReadBuffer,
                                g_BinDIO.readSize);
    }
}


//-----------------------------------------------------------------------------
//
//  Function:  OEMFinishEraseFlash
//
//  This function is called by the BLCOMMON framework when it is about to
//  write the OS image to flash memory.
//
//  Parameters:
//      None.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//-----------------------------------------------------------------------------
BOOL OEMFinishEraseFlash(void)
{
    BOOL rc = TRUE;

    if (g_ImageType == IMAGE_TYPE_BINDIO && g_ImageMemory == IMAGE_MEMORY_NAND)
    {
        rc = NANDFinishWriteBinDIO();
    }

    return rc;
}


//-----------------------------------------------------------------------------
//
//  Function:  OEMWriteFlash
//
//  This function writes to flash memory the OS image that might be stored
//  in a RAM file cache area.
//
//  Parameters:
//      dwStartAddr
//          [in] Address in flash memory where the start of the downloaded
//          OS image is to be written.
//
//      dwLength
//          [in] Length of the OS image, in bytes, to be written to flash
//          memory.
//
//  Returns:
//      TRUE indicates success. FALSE indicates failure.
//-----------------------------------------------------------------------------
BOOL OEMWriteFlash(DWORD dwStartAddr, DWORD dwLength)
{
    BOOL rc = TRUE;
    int key;

    if (g_ImageMemory == IMAGE_MEMORY_NAND && g_ImageType == IMAGE_TYPE_BINDIO)
    {
        goto cleanUp;
    }

	/*
	KITLOutputDebugString("\r\nWARNING:  Flash update requested.\r\n");
    KITLOutputDebugString("Do you want to continue (y/n)? ");

    do {
        key = tolower(OEMReadDebugByte());
    } while ((key != 'y') && (key != 'n'));
    KITLOutputDebugString("\r\n");

    if (key != 'y')
    {
        KITLOutputDebugString("Flash update has been canceled.\r\n");
        rc = FALSE;
        goto cleanUp;
    }
	*/

	//
	// CS&ZHL MAY-21-2011: add for BinFS
	//
	g_dwCurrentBinNum ++;

	// Check if it is the first bin to write
	if(g_dwCurrentBinNum == 1)
	{
	    KITLOutputDebugString("\r\nWARNING: Flash update requested.\r\n");
	    KITLOutputDebugString("Do you want to continue (y/n)? ");

	    do {
	        key = tolower(OEMReadDebugByte());
	    } while ((key != 'y') && (key != 'n'));
	    KITLOutputDebugString("\r\n");

	    if (key != 'y')
	    {
	        KITLOutputDebugString("Flash update has been canceled.\r\n");
	        rc = FALSE;
			g_dwCurrentBinNum = g_dwTotalBinNum;
	        goto cleanUp;
	    }

		if((g_ImageMemory == IMAGE_MEMORY_NAND) && (g_ImageType == IMAGE_TYPE_NK))
		{
			// Write the MBR first
			if(NANDMakeMBR() == FALSE)
			{
		        rc = FALSE;
				g_dwCurrentBinNum = g_dwTotalBinNum;
		        goto cleanUp;
			}
		}
	}



    /*
	if (g_ImageMemory == IMAGE_MEMORY_NOR)
    {
        rc = NORWrite(dwStartAddr, dwLength);
    }
	*/

	if (g_ImageMemory == IMAGE_MEMORY_NAND)
    {
        switch(g_ImageType)
        {
        case IMAGE_TYPE_XLDR:
            rc = NANDWriteXldr(dwStartAddr, dwLength);
            break;

        case IMAGE_TYPE_BOOT:
            rc = NANDWriteSB(dwStartAddr, dwLength);
            break;

        case IMAGE_TYPE_IPL:
            rc = NANDWriteIPL(dwStartAddr, dwLength);
            break;

        case IMAGE_TYPE_NK:
            rc = NANDWriteNK(dwStartAddr, dwLength);
			//
			// CS&ZHL MAY-24-2011: the first bin is XIPKERNEL for booting up
			//
			if(g_dwCurrentBinNum == 1)
			{
				//
				// CS&ZHL MAY-23-2011: save nand flash start address and length into BootCFG if required
				//
				if((g_BootCFG.dwNandImageStart != dwStartAddr) || (g_BootCFG.dwNandImageLength != dwLength))
				{
					g_BootCFG.dwNandImageStart = dwStartAddr;		//virtual start address of OS image in NandFlash
					g_BootCFG.dwNandImageLength = dwLength;		//byte length of OS image in NandFlash
					rc = NANDStoreBootCFG((BYTE *)&g_BootCFG, sizeof(BOOT_CFG));
				}
			}
			break;

        default:
            // Should never get here...
            rc = FALSE;
            break;
        }
    }

    else if (g_ImageMemory == IMAGE_MEMORY_SD)
    {
        switch(g_ImageType)
        {
        case IMAGE_TYPE_XLDR:
            rc = SDHCWriteXldr(dwStartAddr, dwLength);
            break;

        case IMAGE_TYPE_BOOT:
            rc = MX28_SDHCWriteSB(dwStartAddr, dwLength);
            break;

        case IMAGE_TYPE_NK:
            rc = SDHCWriteNK(dwStartAddr, dwLength);
            break;

        default:
            // Should never get here...
            rc = FALSE;
            break;
        }
    }
    else
    {
        // Should never get here...
        rc = FALSE;
    }

cleanUp:
    // For all kinds of images flashed, the system should be re-started
    //KITLOutputDebugString("Reboot the device manually...\r\n");
    //SpinForever();
	// CS&ZHL MAY-21-2011: add for BinFS
	if(g_dwCurrentBinNum == g_dwTotalBinNum)
	{
	    // For all kinds of images flashed, the system should be re-started
	    KITLOutputDebugString("Reboot the device manually...\r\n");

	    SpinForever();
	}

    return rc;
}


BOOL FlashLoadBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{
    BOOL rc;
        
    if (g_bNandBootloader)
    {
        rc = NANDLoadBootCFG(pBootCfg, cbBootCfgSize);
    }
    else if (g_bSDHCBootloader)
    {
        rc = SDHCLoadBootCFG(pBootCfg, cbBootCfgSize);
    }
    else
    {
        rc = NORLoadBootCFG(pBootCfg, cbBootCfgSize);
    }

    return rc;
}


BOOL FlashStoreBootCFG(BYTE *pBootCfg, DWORD cbBootCfgSize)
{
    BOOL rc; 
    
    if (g_bNandBootloader)
    {
        rc = NANDStoreBootCFG(pBootCfg, cbBootCfgSize);
    }
    else if (g_bSDHCBootloader)
    {
        rc = SDHCStoreBootCFG(pBootCfg, cbBootCfgSize);
    }
    else
    {
        rc = NORStoreBootCFG(pBootCfg, cbBootCfgSize);
    }

    return rc;
}
