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
#include <bootpart.h>
#include "bppriv.h"

/*
    @func   BOOL | EraseAllBlocks | Erases all NAND flash blocks regardless of the bad-block marker.
    @rdesc  TRUE returned on success, FALSE on failure.
    @comm
    @xref
*/
extern "C" BOOL EraseAllBlocks(void)
{
    USHORT nBlockNum;

    for (nBlockNum = 0 ; nBlockNum < g_FlashInfo.dwNumBlocks ; nBlockNum++)
    {
        if (!FMD_EraseBlock(nBlockNum))
        {
            return(FALSE);
        }
    }
    return(TRUE);
}


/*
    @func   BOOL | EraseBlocks | Erases a range of NAND flash blocks, tests, and marks bad blocks.
    @rdesc  TRUE on success, FALSE on failure.
    @comm
    @xref
*/
BOOL EraseBlocks(DWORD dwStartBlock, DWORD dwNumBlocks, DWORD dwFlags)
{
    DWORD dwSector;
    USHORT nCount;
    LPBYTE WriteSect = g_pbBlock + g_FlashInfo.wDataBytesPerSector;
    LPBYTE ReadSect = g_pbBlock + 2 * g_FlashInfo.wDataBytesPerSector;
    SectorInfo WriteSectInfo, ReadSectInfo;

    if (dwStartBlock >= g_FlashInfo.dwNumBlocks || (dwStartBlock + dwNumBlocks - 1) >= g_FlashInfo.dwNumBlocks)
    {
        RETAILMSG (1, (TEXT("EraseBlocks: block number outside valid range [0x%x, 0x%x].\r\n"), dwStartBlock, (dwStartBlock + dwNumBlocks - 1)));
        return(FALSE);
    }

    RETAILMSG (1, (TEXT("Erasing flash block(s) [0x%x, 0x%x] (please wait): "), dwStartBlock, (dwStartBlock + dwNumBlocks - 1)));
    while (dwNumBlocks--)
    {
        DWORD dwStatus = FMD_GetBlockStatus (dwStartBlock);
        BOOL fBadBlock = FALSE;

        // If the block has already been marked bad, skip it and increase the total number of blocks to be erased by a block.  Note that bad
        // blocks do count against the total number of blocks to be erased since the caller has max constraints on the erase region size.
        if (dwStatus & BLOCK_STATUS_BAD)
        {
            RETAILMSG (1, (TEXT("EraseBlocks: found a bad block (0x%x) - skipping...\r\n"), dwStartBlock));
            ++dwStartBlock;
            continue;
        }

        if ((dwStatus & BLOCK_STATUS_RESERVED) && (dwFlags & FORMAT_SKIP_RESERVED))
        {
            RETAILMSG (1, (TEXT("EraseBlocks: preserving reserved block (0x%x) \r\n"), dwStartBlock));
            ++dwStartBlock;
            continue;
        }

        if (!FMD_EraseBlock(dwStartBlock))
        {
            RETAILMSG (1, (TEXT("EraseBlocks: unable to erase block (0x%x).  Marking bad..\r\n"), dwStartBlock));
            FMD_SetBlockStatus(dwStartBlock, BLOCK_STATUS_BAD);
            ++dwStartBlock;
            continue;
        }

        // Optionally skip the bad block check - this speeds up the erase process, especially on NOR flash.
        if (dwFlags & FORMAT_SKIP_BLOCK_CHECK)
        {
            ++dwStartBlock;
            continue;
        }

        // Because the bits denoting a bad block can be erased, we take the cautious approach - we'll write an read-verify each sector in this
        // block to make sure the block is good.  If it's good, we'll re-erase else we'll mark the block bad.
        dwSector = (dwStartBlock * g_FlashInfo.wSectorsPerBlock);
        for (nCount = 0 ; nCount < g_FlashInfo.wSectorsPerBlock ; nCount++)
        {
            // Make sure erase set all bits high.
            memset(WriteSect, 0xFF, g_FlashInfo.wDataBytesPerSector);
            memset(&WriteSectInfo, 0xFF, sizeof(SectorInfo));

            FMD_ReadSector((dwSector + nCount), ReadSect, &ReadSectInfo, 1);

            if (memcmp(ReadSect, WriteSect, g_FlashInfo.wDataBytesPerSector) ||
                   memcmp(&ReadSectInfo, &WriteSectInfo, sizeof(SectorInfo)))
            {
                RETAILMSG (1, (TEXT("EraseBlocks: erase didn't set all bits high (marking block 0x%x bad).\r\n"), dwStartBlock));
                FMD_SetBlockStatus(dwStartBlock, BLOCK_STATUS_BAD);
                fBadBlock = TRUE;
                break;
            }

            // Now, make sure we can store zero - this is meant to check for bad blocks (in the event that the bad block marker was erased).
            // Note that we *don't* write sector info data here - this is where bad block data is stored.
            memset(WriteSect, 0, g_FlashInfo.wDataBytesPerSector);
            if (!FMD_WriteSector((dwSector + nCount), WriteSect, NULL, 1))
            {
                RETAILMSG (1, (TEXT("EraseBlocks: write test low data failed (marking block 0x%x bad).\r\n"), dwStartBlock));
                FMD_SetBlockStatus(dwStartBlock, BLOCK_STATUS_BAD);
                fBadBlock = TRUE;
                break;
            }
            // Read back the value and make sure it stored correctly.
            if (!FMD_ReadSector((dwSector + nCount), ReadSect, NULL, 1) ||
                memcmp(ReadSect, WriteSect, g_FlashInfo.wDataBytesPerSector))
            {
                RETAILMSG (1, (TEXT("EraseBlocks: erase didn't set all bits low (marking block 0x%x bad).\r\n"), dwStartBlock));
                FMD_SetBlockStatus(dwStartBlock, BLOCK_STATUS_BAD);
                fBadBlock = TRUE;
                break;
            }
        }

        // If the block has already been marked bad, skip it and increase the total number of blocks to be erased by a block.  Note that bad
        // blocks do count against the total number of blocks to be erased since the caller has max constraints on the erase region size.
        if (fBadBlock)
        {
            RETAILMSG (1, (TEXT("\r\nEraseBlocks: found a bad block (0x%x) - skipping...\r\n"), dwStartBlock));
            ++dwStartBlock;
            continue;
        }

        if (!FMD_EraseBlock(dwStartBlock))
        {
            RETAILMSG (1, (TEXT("EraseBlocks: unable to erase block (0x%x).  Marking bad..\r\n"), dwStartBlock));
            FMD_SetBlockStatus(dwStartBlock, BLOCK_STATUS_BAD);
        }

        ++dwStartBlock;
    }
    RETAILMSG (1, (TEXT("Done.\r\n")));
    return(TRUE);
}

