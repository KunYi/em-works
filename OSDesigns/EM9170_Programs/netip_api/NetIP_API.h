
/*//////////////////////////////////////////////////////////////////////////
Orgnization:  Emtronix Incorporated
Filename:	  NETIP_API.H
Authors:			CS & ZHL
Date:				July, 2008
Description:  defintions of API functions for setting net ip parameters
///////////////////////////////////////////////////////////////////////////*/
#if      !defined(_NETIP_API_H)
#define  _NETIP_API_H

#define  MAXNUMOFADPT	4

//
// ���屾�������������������Ϣ�ṹ
//
typedef struct _NETWORK_ADPT_INFO 
{
	LPTSTR	szAdapterName;				//	����������������
	BOOL	fUseDHCP;					//	�Ƿ�����DHCP��־��TRUE��������DHCP��FALSE�����ر�DHCP
	DWORD	IPAddr;						//	�����������ľ�̬IP��ַ
	DWORD	SubnetMask;					//	��������
	DWORD	Gateway;					//	Ĭ������
	DWORD	DNSAddr;					//	����DNS��������ַ
	DWORD	DNSAltAddr;					//	����DNS��������ַ
	DWORD	WINSAddr;					//	����WINS��������ַ
	DWORD	WINSAltAddr;				//	����WINS��������ַ
	TCHAR	szDisplayName[256];			//	��������������ʾ����
} NETWORK_ADPT_INFO, *PNETWORK_ADPT_INFO;

//
// ���屾�����������������������������Ϣ��ؽṹ�����֧��4����������
//
typedef struct _NETWORK_ADPTS_NAME 
{
	TCHAR	szAdapterName[MAXNUMOFADPT][64];	//	���б�����������������������
	DWORD   NumOfAdapters;						//	ʵ�ʱ����������Ŀ
} NETWORK_ADPTS_NAME, *PNETWORK_ADPTS_NAME;

///////////////////////////////////////////////////////////////////////////////////////
// ����������
//    ��IP��ַ�ַ�����ʽת��ΪDWORDֵ����NETWORK_ADPT_INFO����Ҫ����IP��ַ��DWORDֵ��
//           
//
// ������� 
//    IPAddressString:  IP��ַ���ַ�������: _T("192.168.201.182")
//
// �������
//    IPAddressValue:  IP��ַ��DWORDֵ��
//
// ����ֵ = TRUE:	�����ɹ�
//        = FALSE:	����ʧ��
///////////////////////////////////////////////////////////////////////////////////////
BOOL  StringToIPAddr(TCHAR *IPAddressString, DWORD *IPAddressValue);

///////////////////////////////////////////////////////////////////////////////////////
// ����������
//    ��ȡ��������ĸ����Լ���Ӧ�ĸ������������������ơ�
//           
// �������
//    pAdptsName:  ��ȡ�õ���NETWORK_ADPTS_NAME�ṹ������
//
// ����ֵ = TRUE:	�����ɹ�
//        = FALSE:	����ʧ��
///////////////////////////////////////////////////////////////////////////////////////
BOOL  GetNetWorkAdaptersName( PNETWORK_ADPTS_NAME pAdptsName );

///////////////////////////////////////////////////////////////////////////////////////
// ����������
//    ��ȡ������������������ز���������IP���������롢���صȡ�
//           
// �������
//    pAdptInfo:  ��ȡ�õ���NETWORK_ADPT_INFO�ṹ������
//
// ����ֵ = TRUE:	�����ɹ�
//        = FALSE:	����ʧ��
///////////////////////////////////////////////////////////////////////////////////////
BOOL  GetNetWorkAdapterInfo( LPTSTR szAdapterName, PNETWORK_ADPT_INFO pAdptInfo );

///////////////////////////////////////////////////////////////////////////////////////
// ����������
//    ���ñ�����������������ز���������IP���������롢���صȡ�
//           
// �������
//    pAdptInfo:  ��Ҫ���õ�NETWORK_ADPT_INFO�ṹ������
//
// ����ֵ = TRUE:	�����ɹ�
//        = FALSE:	����ʧ��
///////////////////////////////////////////////////////////////////////////////////////
BOOL  SetNetWorkAdapterInfo( LPTSTR szAdapterName, PNETWORK_ADPT_INFO pAdptInfo );

#endif