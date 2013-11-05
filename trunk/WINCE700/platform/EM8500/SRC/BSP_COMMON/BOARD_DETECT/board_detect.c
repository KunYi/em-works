/*
===============================================================================
*             Texas Instruments OMAP(TM) Platform Software
* (c) Copyright Texas Instruments, Incorporated. All Rights Reserved.
*
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied.
*
===============================================================================
*/
//
//  File:  board_detect.c
//
#include "bsp.h"
#include "sdk_i2c.h"
#include "am33x_clocks.h"


//-----------------------------------------------------------------------------
//
//  Global:  g_dwBoardId
//
//  Set during OEMInit to indicate Board ID read from EEPROM.
//

DWORD g_dwBoardId = (DWORD)AM33X_BOARDID_GP_BOARD;

//-----------------------------------------------------------------------------
//
//  Global:  g_dwBoardProfile
//
//  Set during OEMInit to indicate Board Profile read from CPLD.
//

DWORD g_dwBoardProfile = (DWORD)PROFILE_0;

//-----------------------------------------------------------------------------
//
//  Global:  g_dwBoardHasDcard
//
//  Set during OEMInit to indicate if board has daugther board.
//

DWORD g_dwBoardHasDcard = (DWORD)TRUE;

//-----------------------------------------------------------------------------
//
//  Global:  g_dwBaseBoardVersion
//
//  Set during OEMInit to indicate if board has daugther board.
//

DWORD g_dwBaseBoardVersion = (DWORD)AM335X_BOARD_VERSION_1_1_A;



#define NO_OF_MAC_ADDR          3
#define ETH_ALEN		        6

/* CPLD registers */
#define CFG_REG			0x10

/*
 * I2C Address of various board
 */
#define I2C_BASE_BOARD_ADDR	0x50
#define I2C_DAUGHTER_BOARD_ADDR 0x51
#define I2C_LCD_BOARD_ADDR	0x52

#define I2C_CPLD_ADDR		0x35


struct am335x_baseboard_id {
	unsigned int  magic;
	char name[8];
	char version[4];
	char serial[12];
	char config[32];
    char mac_addr[NO_OF_MAC_ADDR][ETH_ALEN];
};


struct am335x_daughterBoard_id {
	unsigned int  magic;
	char name[8];
	char version[4];
	char serial[12];
	char config[32];
};

/*
 * Daughter board detection: All boards have i2c based EEPROM in all the
 * profiles. Base boards and daughter boards are assigned unique I2C Address.
 * We probe for daughter card and then if sucessful, read the EEPROM to find
 * daughter card type.
 */
 static BOOL detect_daughter_board(void * m_hI2CDevice)
{   
    static struct am335x_daughterBoard_id db_header;    
    
    g_dwBoardHasDcard=TRUE;

    if (m_hI2CDevice == NULL) goto cleanUp;
    
    /* read header from daughter board */
    I2CSetSlaveAddress(m_hI2CDevice, (UINT16)I2C_DAUGHTER_BOARD_ADDR);    
    I2CSetSubAddressMode(m_hI2CDevice,I2C_SUBADDRESS_MODE_16);    
    
    if (I2CRead(m_hI2CDevice, 0, (void *)&db_header, sizeof(db_header))!=(UINT)-1) {
        if (db_header.magic == 0xEE3355AA) {	
            OALMSG(1,(L"Daughter Board is present\r\n"));        
		    g_dwBoardHasDcard = TRUE;
            OALMSG(1,(L"\tDB Board Name: %.8S\r\n",db_header.name));
            OALMSG(1,(L"\tDB Board Ver : %.4S\r\n",db_header.version));
            OALMSG(1,(L"\tDB Board Ser : %.12S\r\n",db_header.serial));
            OALMSG(1,(L"\tDB Board Type: %.6S\r\n",db_header.config));
	    }
        else {
            OALMSG(1,(L"DAUGHTER BOARD: read wrong magic value 0x%x\r\n",db_header.magic));
            g_dwBoardHasDcard = FALSE;
        }
    }
    else {
        OALMSG(1,(L"DAUGHTER BOARD: Cannot read DB EEPROM\r\n"));
        g_dwBoardHasDcard = FALSE;
    }
cleanUp:
    return g_dwBoardHasDcard;

}

static void detect_daughter_board_profile(void * m_hI2CDevice)
{
    
	DWORD val = 0;
    
	if (m_hI2CDevice == NULL) return;
    
    I2CSetSlaveAddress(m_hI2CDevice, (UINT16)I2C_CPLD_ADDR);        
    if (I2CRead(m_hI2CDevice, CFG_REG, (void *)(&val), 2)!=(UINT)-1)    {        
	    g_dwBoardProfile = 1 << (val & 0x7);
        OALMSG(1,(L"Using Profile#%d (g_dwBoardProfile=0x%x)\r\n",(val & 0x7),g_dwBoardProfile));
    }
    else
        OALMSG(1,(L"DAUGHTER BOARD: Cannot read CPLD; using default Profile#0 (g_dwBoardProfile=0x%x)\r\n",g_dwBoardProfile));

}

/* caller should have called OALI2CInit and I2CPostinit before calling this function */
/* Assumes that the pinmux for the I2C0 is already set before this function is called */
BOOL detect_board_id_and_profile_info()
{
    void * m_hI2CDevice = NULL;
    
    static struct am335x_baseboard_id header;
    unsigned short val;
    DWORD readBytes=0;

    // open i2c device
    m_hI2CDevice = I2COpen(AM_DEVICE_I2C0);  
    if (m_hI2CDevice == NULL)
	{
		OALMSG(1, (L"AM33x:OALPowerPostInit: Failed to open I2C driver"));
		return FALSE;
	}    
    
    /* read header from base board */
    I2CSetSlaveAddress(m_hI2CDevice, (UINT16)I2C_BASE_BOARD_ADDR);
    I2CSetBaudIndex(m_hI2CDevice,SLOWSPEED_MODE);
    I2CSetSubAddressMode(m_hI2CDevice,I2C_SUBADDRESS_MODE_16);
    if ((readBytes=I2CRead(m_hI2CDevice, 0, (void *)&header, sizeof(header)))==(UINT)-1) {
        OALMSG(1, (L"Cannot read base board I2C EEPROM\r\n"));
        return FALSE;
    }
    if (header.magic != 0xEE3355AA) {		
        OALMSG(1, (L"I2C EEPROM returned wrong magic value 0x%x\r\n",header.magic));
		return FALSE;
	}   
    
    /* determine the board type and then read profile */
    if (!strncmp("SKU#00", header.config, 6)) {
        OALMSG(1, (L"IT IS A BASE BOARD\r\n"));
        g_dwBoardId = AM33X_BOARDID_BASE_BOARD;     
    }
    else if (!strncmp("SKU#01", header.config, 6)) {
        OALMSG(1, (L"IT IS A GP BOARD \r\n"));
        g_dwBoardId = AM33X_BOARDID_GP_BOARD;           
    }
    else if (!strncmp("SKU#02", header.config, 6)) {
        OALMSG(1, (L"IT IS A IA BOARD\r\n"));
        g_dwBoardId = AM33X_BOARDID_IA_BOARD;           
    }
    else if (!strncmp("SKU#03", header.config, 6)) {
        OALMSG(1, (L"IT IS A IPP BOARD\r\n"));
        g_dwBoardId = AM33X_BOARDID_IPP_BOARD;
    }
    else {
        OALMSG(1, (L"IT IS A INVALID BOARD\r\n"));
        return FALSE;
    }  

    if (!strncmp("1.0A", header.version, 4)) {        
        g_dwBaseBoardVersion = AM335X_BOARD_VERSION_1_0_A;     
    }
    else if (!strncmp("1.0B", header.version, 4)) {        
        g_dwBaseBoardVersion = AM335X_BOARD_VERSION_1_0_B;     
    }
    else if (!strncmp("1.1A", header.version, 4)) {        
        g_dwBaseBoardVersion = AM335X_BOARD_VERSION_1_1_A;     
    }
    else
        OALMSG(1, (L"Unrecognized Board Version!\r\n"));

    //print the Base board info    
    OALMSG(1,(L"\tBoard Name: %.8S\r\n",header.name));
    OALMSG(1,(L"\tBoard Ver : %.4S\r\n",header.version));
    OALMSG(1,(L"\tBoard Ser : %.12S\r\n",header.serial));
    OALMSG(1,(L"\tBoard Type: %.6S\r\n",header.config));
    
    //detect daughter board
    if ((detect_daughter_board(m_hI2CDevice))&&
        ((g_dwBoardId == AM33X_BOARDID_GP_BOARD) || (g_dwBoardId == AM33X_BOARDID_IA_BOARD))) 
    {
        detect_daughter_board_profile(m_hI2CDevice);        
    }    
 
    I2CSetBaudIndex(m_hI2CDevice,FULLSPEED_MODE);
    I2CSetSubAddressMode(m_hI2CDevice,I2C_SUBADDRESS_MODE_8);       
    

    I2CClose(m_hI2CDevice);

    return TRUE;
}


