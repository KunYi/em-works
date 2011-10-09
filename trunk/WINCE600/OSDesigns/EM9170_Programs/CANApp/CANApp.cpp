#include <windows.h>
#include "canbus.h"
#include "pkfuncs.h"

typedef enum
{
    SENDER,
    RECEIVER,
    UNDEFINED
} APP_FUNCTION;


void DisplayUsage(void);
BOOL ParseArgument(TCHAR * arg, APP_FUNCTION* appFunction);
BOOL StartCanSender(void);
BOOL StartCanReceiver(void);

DWORD GetSOCCanBusIndex()
{
    PROCESSOR_INFO pi;
    DWORD dwBytesReturned;
    DWORD CanBusIndex=1;
    
    if (!KernelIoControl(IOCTL_PROCESSOR_INFORMATION, NULL, 0, &pi, sizeof(PROCESSOR_INFO), &dwBytesReturned)) 
    {
        ERRORMSG(TRUE, (TEXT("KernelIoContro Get PROCESSOR Info Fail! \r\n"))); 
        return 1; 
    }
    RETAILMSG(1,(TEXT("PROCESSOR  = %s\r\n"), pi.szProcessorName));

    if(0==wcscmp(pi.szProcessorName,TEXT("MX25")))
        CanBusIndex= 2;
    else if(0==wcscmp(pi.szProcessorName,TEXT("MX35")))
        CanBusIndex= 1;
    else
        CanBusIndex= 1;

    return CanBusIndex;

}    


int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
    APP_FUNCTION appFunction = UNDEFINED;

     UNREFERENCED_PARAMETER(envp);

    _tprintf(_T("CANApp: CAN Test Application \n"));

    if (argc != 2)
    {
        _tprintf(_T("Missing argument\n"));
        DisplayUsage();
        goto exit;
    }

    if (ParseArgument(argv[1], &appFunction) == FALSE)
    {
        DisplayUsage();
        goto exit;
    }

    switch(appFunction)
    {
    case SENDER:
        StartCanSender();
        break;
    case RECEIVER:
        StartCanReceiver();
        break;
    default:
        DisplayUsage();
        goto exit;
    }

exit:
    return 0;
}

BOOL ParseArgument(TCHAR * arg, APP_FUNCTION* appFunction)
{
    TCHAR * pCurrentChar;
    BOOL functionFind = FALSE;

    pCurrentChar = arg;

    if (*pCurrentChar != '-')
    {
        return FALSE;
    }
    pCurrentChar++;
    if ((*pCurrentChar == 's') || (*pCurrentChar == 'S'))
    {
        *appFunction = SENDER;
        functionFind = TRUE;
    }
    if ((*pCurrentChar == 'r') || (*pCurrentChar == 'R'))
    {
        *appFunction = RECEIVER;
        functionFind = TRUE;
    }    
    pCurrentChar++;
    if ((*pCurrentChar == NULL) && (functionFind == TRUE))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL StartCanSender(void)
{
    HANDLE hCan = INVALID_HANDLE_VALUE;
    TCHAR devname[5];
    CAN_PACKET cp = {0};
    CAN_TRANSFER_BLOCK ctb = {0};
    BYTE data[8];
    BYTE dwStIdx = 48;
    BOOL bRet;
    int ret;

   // swprintf(devname,L"CAN%x:",GetSOCCanBusIndex());
	ret = 2;
    swprintf(devname,L"CAN%x:",ret);
    hCan = CANOpenHandle(devname);
    if(hCan == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Error ont CANOpenHandle\n"));
        return FALSE;
    }
    _tprintf(_T("CAN frame send start\n"));

    CANSetMode(hCan, dwStIdx, CAN_TX);  //set can index as transmitte or receiver mode 

    data[0] = 0x00;
    data[1] = 0x12;
    data[2] = 0x34;
    data[3] = 0x56;
    data[4] = 0x78;
    data[5] = 0x90;
    data[6] = 0x00;
    data[7] = 0xFF;

    //_tprintf(_T("  Start send data on CAN port %x, id 0x1234\n"),GetSOCCanBusIndex());
    _tprintf(_T("  Start send data on CAN port %x, id 0x1234\n"),ret);

    for( BYTE i=0;i<100;i++)
    {
        ret = 0;

        data[0] = i;

        cp.byIndex = (BYTE)dwStIdx;
        cp.byRW = CAN_RW_WRITE;
        cp.fromat = CAN_STANDARD;   //CAN_EXTENDED;
        cp.frame =CAN_DATA;
        cp.ID = 0x03;
        cp.wLen = 8;
        cp.pbyBuf = (PBYTE)data;
        cp.lpiResult = &ret;
        ctb.pCANPackets = &cp;
        ctb.iNumPackets = 1;
        
        _tprintf(_T("%i) Send CAN port: DATA 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n"),i, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

        bRet = CANTransfer(hCan, &ctb);
       
        if (bRet == FALSE)
        {
            _tprintf(_T("%i) Error: CANTransfer return FALSE\n"),i);
        }
        else if (ret != CAN_NO_ERROR)
        {
            _tprintf(_T("%i) Error: ret value is %d\n"),i,ret);
        }
        else
        {
            _tprintf(_T("%i) Send success\n"),i);
        }

        Sleep(1000);
    }

    CANCloseHandle(hCan);

    return TRUE;
}

BOOL StartCanReceiver(void)
{
    HANDLE hCan = INVALID_HANDLE_VALUE;
    TCHAR devname[5];
    CAN_PACKET cp = {0};
    CAN_TRANSFER_BLOCK ctb = {0};
    BYTE data[8];
    BYTE dwRdIdx = 47;
    BOOL bRet;
    int ret;

   // swprintf(devname,L"CAN%x:",GetSOCCanBusIndex());
	ret = 1;
    swprintf(devname,L"CAN%x:",ret);
    hCan = CANOpenHandle(devname);
    if(hCan == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Error ont CANOpenHandle\n"));
        return FALSE;
    }

    //_tprintf(_T("  Start receive data on CAN port %x, id 0x1234\n"),GetSOCCanBusIndex());
    _tprintf(_T("  Start receive data on CAN port %x, id 0x1234\n"),ret);

    CANSetMode(hCan, dwRdIdx,CAN_RX);  //set can index as transmitte or receiver mode 

    for( BYTE i=0;i<20;i++)
    {
        data[0] = 0x00;
        data[1] = 0x00;
        ret = 0;

        cp.byIndex = (BYTE)dwRdIdx;
        cp.byRW = CAN_RW_READ;
        cp.fromat =  CAN_STANDARD;   //CAN_EXTENDED; 
        cp.frame =CAN_DATA;
        cp.ID = 0x03;
        cp.wLen = 8;
        cp.pbyBuf = (PBYTE)data;
        cp.lpiResult = &ret;
        ctb.pCANPackets = &cp;
        ctb.iNumPackets = 1;

        bRet = CANTransfer(hCan, &ctb); //read and write data 
               
        if (bRet == FALSE)
        {
            _tprintf(_T("%i) Error: CANTransfer return FALSE\n"),i);
        }
        else if (ret != CAN_NO_ERROR)
        {
            _tprintf(_T("%i) Error: error code value is %d\n"),i, ret);
        }
        else
        {
            _tprintf(_T("%i) Received CAN port: DATA 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n"),i, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
        }

        Sleep(100);
    }

    CANCloseHandle(hCan);

    return TRUE;
}

void DisplayUsage(void)
{
    _tprintf(_T("--------------------------------------\n"));
    _tprintf(_T("Command line usage:\n"));
    _tprintf(_T(" -s : For send CAN message\n"));
    _tprintf(_T(" -r : For receive CAN message\n"));
    _tprintf(_T("--------------------------------------\n"));
}
