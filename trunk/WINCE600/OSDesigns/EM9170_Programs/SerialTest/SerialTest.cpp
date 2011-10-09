// SerialTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"


#define BUFFER_SIZE     0x200

HANDLE hSerialPort;


//****************************************************************************
//
// OpenPort opens the specified serial port.
//
//****************************************************************************
int
OpenPort(long lPort)
{
    WCHAR pcName[16];

    //
    // Create the device name for the given serial port.
    //
    wsprintf(pcName, L"COM%d:", lPort);

    //
    // Open the serial port.
    //
    hSerialPort = CreateFile(pcName, GENERIC_READ | GENERIC_WRITE, 0, 0,
                             OPEN_EXISTING, 0, 0);
    if(hSerialPort == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Could not open serial port %s.\n", pcName);
		RETAILMSG(1, (L"Could not open serial port %s.\n", pcName));
        return(0);
    }

    //
    // Success.
    //
    return(1);
}

//****************************************************************************
//
// SetBaud sets the baud rate and data format of the serial port.
//
//****************************************************************************
void
SetBaud( long lRate )
{
    DCB dcb;

    //
    // Purge any pending characters in the serial port.
    //
    PurgeComm(hSerialPort, (PURGE_TXABORT | PURGE_RXABORT |
                            PURGE_TXCLEAR | PURGE_RXCLEAR));

    //
    // Fill in the device control block.
    //
    dcb.DCBlength = sizeof(DCB);
    dcb.BaudRate = lRate;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = TRUE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fAbortOnError = FALSE;
    dcb.XonLim = 0;
    dcb.XoffLim = 0;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.XonChar = 17;
    dcb.XoffChar = 19;
    dcb.ErrorChar = 0;
    dcb.EofChar = 0;
    dcb.EvtChar = 0;
    dcb.wReserved = 0;

    //
    // Set the new serial port configuration.
    //
    SetCommState(hSerialPort, &dcb);
}

//****************************************************************************
//
// SendChar sends a character to the serial port.
//
//****************************************************************************
void  SendChar(char cChar)
{
    DWORD dwLen;

    //
    // Send this character to the serial port.
    //
    WriteFile(hSerialPort, &cChar, 1, &dwLen, NULL);
}

DWORD SerialSend( char* Buf, int len )
{
    DWORD dwLen;

    //
    // Send this character to the serial port.
    //
    WriteFile(hSerialPort, Buf, len, &dwLen, NULL);
	return dwLen;
}
//****************************************************************************
//
// ReceiveChar reads a character from the serial port.
//
//****************************************************************************
char
ReceiveChar(long lTimeout)
{
    COMMTIMEOUTS sTimeouts;
    char cChar;
    DWORD dwLen;

    //
    // Fill in the timeout structure based on the timeout requested for this
    // read.
    //
    sTimeouts.ReadIntervalTimeout = 0;
    sTimeouts.ReadTotalTimeoutMultiplier = 0;
    sTimeouts.ReadTotalTimeoutConstant = lTimeout;
    sTimeouts.WriteTotalTimeoutMultiplier = 0;
    sTimeouts.WriteTotalTimeoutConstant = 0;

    //
    // Set the timeout for this read.
    //
    SetCommTimeouts(hSerialPort, &sTimeouts);

    //
    // Read a character.
    //
    if(!ReadFile(hSerialPort, &cChar, 1, &dwLen, NULL))
    {
        //
        // The read failed, so set the read character to a NULL.
        //
        cChar = 0;
    }

    //
    // If we did not read a character, then set the character to NULL.
    //
    if(dwLen != 1)
    {
        cChar = 0;
    }

    //
    // Return the character we read.
    //
    return(cChar);
}

int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
    // TODO: Place code here.
	//
    // Open the serial port and set the Baud rate.
    //
	int			i1, len, PortNum, Baud;
	BOOL		bResult;
	ULONG  WaitReturn;
	char		BufStr[100];
	char       byte;

	printf( "Serial Test Program V1.0\n" );
	PortNum = 2;
	Baud = 115200;
	i1 = wcslen( lpCmdLine );
	if( i1>0 )
	{
		wcstombs( BufStr, lpCmdLine, i1 );
		sscanf( BufStr, "%d %d", &PortNum, &Baud );
	}

	printf( "PortNum: %d Baud: %d\r\n", PortNum, Baud );

	OpenPort(PortNum);
    SetBaud(Baud);

	strcpy( BufStr, "Emtronix EM9170 Serial Test!");
 	len = strlen( BufStr );
    //for(i1 = 0; i1< len ; i1++)
   // for( ; ; )
    {
		SerialSend( BufStr, len );
		Sleep( 100 );
    }


    for(i1=0; ;)
    {
		BufStr[i1] = ReceiveChar( 10 );
		if( BufStr[i1]!=0 )
			i1++;
		if( (BufStr[i1-1]=='!')||(i1>=100) )
		{
			printf( "RCV:%d\n", i1 );
			SerialSend( BufStr, i1 );
			i1=0;
		}
    }
    
    
    CloseHandle(hSerialPort);
    return 0;
}



