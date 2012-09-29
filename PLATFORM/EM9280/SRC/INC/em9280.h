#ifndef __EM9280_H
#define __EM9280_H

#if __cplusplus
extern "C" {
#endif

typedef struct
{
	WORD	wFlag;					// => 0x55AA;
	BYTE	CrytoKey[64];
	BYTE	CrytoCode[60];
	DWORD	dwActualLength;
	UCHAR	CopyRightString[64];
	// 1 block <-> 1-bit, support total 4096 blocks
	BYTE	BadBlockTable[512];		// -> = 1: Bad Block, = 0: normal block
	BYTE	ucPading;
	BYTE	ucCheckSum;
} VENDOR_SECURITY_INFO, *PVENDOR_SECURITY_INFO;

#if __cplusplus
}
#endif

#endif	//__EM9280_H
