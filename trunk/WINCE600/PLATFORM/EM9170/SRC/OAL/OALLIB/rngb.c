//-----------------------------------------------------------------------------
//
// Copyright (C) 2007-2008, Freescale Semiconductor, Inc. All Rights Reserved.
// THIS SOURCE CODE, AND ITS USE AND DISTRIBUTION, IS SUBJECT TO THE TERMS
// AND CONDITIONS OF THE APPLICABLE LICENSE AGREEMENT
//
//-----------------------------------------------------------------------------
//
// File: rngb.c
//
// Implementation for seed generations and random number generation.
//  
// Note: Automatic Seeding Operation is used, and needn't seed input.
//
//-----------------------------------------------------------------------------
#include <bsp.h>

//-----------------------------------------------------------------------------
// External Functions
extern VOID OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX index,
    DDK_CLOCK_GATE_MODE mode);

//-----------------------------------------------------------------------------
// External Variables


//-----------------------------------------------------------------------------
// Defines

//-----------------------------------------------------------------------------
// Local Functions
UINT32 RNGB_GetRandomData(BYTE *pBuf, UINT32 dwNumBytes);

//-----------------------------------------------------------------------------
//
// Function: RNGB_Init
//
// This function enables the RNGB if available.
//
// Note: Once enabled the RGNC cannot be stopped until a hardware reset 
// or secure mode change
//
// Parameters:
//      None.
//
// Returns:
//      Returns TRUE if RNGB is present & FALSE if it is not present.
//
//-----------------------------------------------------------------------------

BOOL RNGB_Init(void)
{

    PCSP_RNGB_REGS pRNGBReg;

    pRNGBReg = (PCSP_RNGB_REGS) OALPAtoVA((CSP_BASE_REG_PA_RNGB), FALSE);

    // Enable clock to RNGB module
    OALClockSetGatingMode(DDK_CLOCK_GATE_INDEX_RNGB,
                          DDK_CLOCK_GATE_MODE_ENABLED);
    //  Check the RNG type is RNGB
    if(RNGB != EXTREG32BF(&pRNGBReg->RNG_ID,RNG_ID_TYPE))
    {
        OALMSG(OAL_ERROR, (L"ERROR: Failed to Read RNGB ID\r\n"));
        return FALSE;
    }

    // Software reset
    INSREG32BF(&pRNGBReg->RNG_COMMAND, RNG_COMMAND_SWRST, RNG_COMMAND_SWRST_SET);

    // Waiting for RNGB into Sleep Mode
    while(!EXTREG32BF(&pRNGBReg->RNG_SR, RNG_SR_SLEEP));
    // Maybe should check secure state to present the completion of software reset  >>> need debug
    //while(!EXTREG32BF(&pRNGBReg->RNG_SR, RNG_SR_SECSTAT));
    
    // Set FIFO_UFLOW_RESPONSE
    INSREG32BF(&pRNGBReg->RNG_CR, RNG_CR_FIFORES, RNG_CR_FIFORES_CLEAR);
    // Mask error interrupt
    INSREG32BF(&pRNGBReg->RNG_CR,RNG_CR_MASKERR, RNG_CR_MASKERR_SET);
    // Mask interrupt
    INSREG32BF(&pRNGBReg->RNG_CR,RNG_CR_MASKDONE, RNG_CR_MASKDONE_SET);
    // Clear error
    INSREG32BF(&pRNGBReg->RNG_COMMAND,RNG_COMMAND_CLRERR, RNG_COMMAND_CLRERR_SET);
    // Clear interrupt
    INSREG32BF(&pRNGBReg->RNG_COMMAND,RNG_COMMAND_CLRINT, RNG_COMMAND_CLRINT_SET);
    // Generate seed
    INSREG32BF(&pRNGBReg->RNG_COMMAND,RNG_COMMAND_SEED, RNG_COMMAND_SEED_SET);
    
    return TRUE;

}

//-----------------------------------------------------------------------------
//
// Function: OALIoCtlHalGetRandomSeed
//
// Implements the IOCTL_HAL_GET_RANDOM_SEED handler.
//
// Parameters:
//      dwIoControlCode
//          [in] The control code specifying the command to execute.
//
//      lpInBuf
//          [in] Long pointer to a buffer that contains the data required to
//          perform the operation. Set to NULL if the dwIoControlCode parameter
//          specifies an operation that does not require input data.
//
//      nInBufSize
//          [in] Size, in bytes, of the buffer pointed to by pInBuf.
//
//      lpOutBuf
//          [out] Long pointer to a buffer that receives the output data for
//          the operation. Set to NULL if the dwIoControlCode parameter
//          specifies an operation that does not produce output data.
//
//      nOutBufSize
//          [in] Size, in bytes, of the buffer pointed to by pOutBuf.
//
//      lpBytesReturned
//          [out] Long pointer to a variable that receives the size, in bytes,
//          of the data stored into the buffer pointed to by pOutBuf. Even
//          when an operation produces no output data and pOutBuf is set to
//          NULL, the DeviceIoControl function uses the variable pointed to
//          by pBytesReturned. After such an operation, the value of the
//          variable has no meaning.
//
// Returns:
//      Returns TRUE if Successfull & FALSE upon error.
//
//-----------------------------------------------------------------------------
BOOL OALIoCtlHalGetRandomSeed(UINT32 dwIoControlCode, VOID *lpInBuf, 
    UINT32 nInBufSize, VOID *lpOutBuf, UINT32 nOutBufSize, UINT32* lpBytesReturned)
{
    BOOL ret = FALSE;
    
    // Remove-W4: Warning C4100 workaround
    UNREFERENCED_PARAMETER(dwIoControlCode);
    UNREFERENCED_PARAMETER(lpInBuf);
    UNREFERENCED_PARAMETER(nInBufSize);
    UNREFERENCED_PARAMETER(lpOutBuf);
    UNREFERENCED_PARAMETER(nOutBufSize);
    UNREFERENCED_PARAMETER(lpBytesReturned);

    *lpBytesReturned = RNGB_GetRandomData((BYTE *)lpOutBuf, nOutBufSize);
    ret = ((*lpBytesReturned) > 0);

    return ret;
}

//-----------------------------------------------------------------------------
//
// Function: RNGB_GetRandomData
//
// Retrieves the desired amount of random data from RNGB.

// Note: this function will block until the desired amount of data is retrieved
//
// Parameters:
//      pBuf
//          [in] pointer to a buffer that receives the output data for
//          the operation.
//
//      dwNumBytes
//          [in] Size, in bytes, of the number of random bytes to get.
//
// Returns:
//      the number of read bytes.
//
//-----------------------------------------------------------------------------
UINT32 RNGB_GetRandomData(BYTE *pBuf, UINT32 dwNumBytes )
{
    PCSP_RNGB_REGS pRNGBReg;
    UINT32 tempRand32;
    UINT32 bytesReqd;
    UINT32 minBytes;

    OALMSG(1, (L"->RNGB_GetRandomData\r\n"));
    pRNGBReg = (PCSP_RNGB_REGS) OALPAtoVA((CSP_BASE_REG_PA_RNGB), FALSE);
    if(pRNGBReg == NULL)
    {
        //OALMSG(OAL_ERROR, (L"RNGB_GetRandomData\r\n"));
		OALMSG(1, (L"RNGB_GetRandomData: pRNGBReg = NULL!\r\n"));
    }
    
    // Clear error
    INSREG32BF(&pRNGBReg->RNG_COMMAND,RNG_COMMAND_CLRERR, RNG_COMMAND_CLRERR_SET);
    // Clear interrupt
    INSREG32BF(&pRNGBReg->RNG_COMMAND,RNG_COMMAND_CLRINT, RNG_COMMAND_CLRINT_SET);
    // Setup RNGB into Automatic Seeding Mode
    INSREG32BF(&pRNGBReg->RNG_CR, RNG_CR_AUTOSEED, RNG_CR_AUTOSEED_SET);
    // Generate seed
    INSREG32BF(&pRNGBReg->RNG_COMMAND,RNG_COMMAND_SEED, RNG_COMMAND_SEED_SET);

    // Wait for the first seeding completion
    //while(!EXTREG32BF(&pRNGBReg->RNG_SR, RNG_SR_SEEDDONE));
    //bytesReqd = dwNumBytes;
	//
	// CS&ZHL JUN-1-2011: get seed if possible
	//
    bytesReqd = 0;
	for( tempRand32 = 0;  tempRand32 < 1000; tempRand32++)
	{
		if(EXTREG32BF(&pRNGBReg->RNG_SR, RNG_SR_SEEDDONE))
		{
			bytesReqd = dwNumBytes;
			break;
		}
	}

	if(bytesReqd == 0)
	{
		// Software reset
		INSREG32BF(&pRNGBReg->RNG_COMMAND, RNG_COMMAND_SWRST, RNG_COMMAND_SWRST_SET);
		OALMSG(1, (L"RNGB_GetRandomData return 0\r\n"));
		return 0;
	}

    while (bytesReqd > 0)
    {
  
        if (EXTREG32BF(&pRNGBReg->RNG_SR, RNG_SR_FIFOLVL))
        {
            tempRand32 = INREG32(&pRNGBReg->RNG_OFIFO);
            minBytes = (bytesReqd >= sizeof(tempRand32)) ? sizeof(tempRand32) : bytesReqd;
            memcpy (pBuf, &tempRand32, minBytes);
            bytesReqd -= minBytes;
            pBuf += minBytes;
        }
       
    }

    // Software reset
    INSREG32BF(&pRNGBReg->RNG_COMMAND, RNG_COMMAND_SWRST, RNG_COMMAND_SWRST_SET);

    return dwNumBytes;
}
