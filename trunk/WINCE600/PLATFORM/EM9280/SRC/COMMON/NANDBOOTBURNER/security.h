#ifndef __EM9280_SECURITY_H
#define __EM9280_SECURITY_H

#if __cplusplus
extern "C" {
#endif

typedef struct
{
	WORD	wFlag;							// => 0x55AA;
	UCHAR	CopyRightString[128];
	BYTE	CrytoKey[64];
    union 
	{
		BYTE	VendorInfoZone[64];
        struct 
		{
			DWORD	dwNumOfPort;			// = 1, 2, 3, 4....
			WORD	mac[4];					// Increase from 3 to 4 for pad purpose
			DWORD	dwYear;					// CS&ZHL APR-10-2012: 
			DWORD	dwMonth;				// CS&ZHL APR-10-2012: 1 - 12
			DWORD	dwDay;					// CS&ZHL APR-10-2012: 1 - 31
        };
    };
	// 1 block <-> 1-bit, support total 4096 blocks
	BYTE	BadBlockTable[512];				// -> = 1: Bad Block, = 0: normal block
	DWORD	dwNumOfBlocks;
	BYTE	ucPadding[1];
	BYTE	ucCheckSum;
} VENDOR_SECURITY_INFO, *PVENDOR_SECURITY_INFO;

#if __cplusplus
}
#endif

#endif	//__EM9280_SECURITY_H
