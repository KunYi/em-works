
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
// 定义本地网络适配器的相关信息结构
//
typedef struct _NETWORK_ADPT_INFO 
{
	LPTSTR	szAdapterName;				//	网络适配器的名称
	BOOL	fUseDHCP;					//	是否启动DHCP标志，TRUE表明启动DHCP，FALSE表明关闭DHCP
	DWORD	IPAddr;						//	网络适配器的静态IP地址
	DWORD	SubnetMask;					//	子网掩码
	DWORD	Gateway;					//	默认网关
	DWORD	DNSAddr;					//	首先DNS服务器地址
	DWORD	DNSAltAddr;					//	备用DNS服务器地址
	DWORD	WINSAddr;					//	首先WINS服务器地址
	DWORD	WINSAltAddr;				//	备用WINS服务器地址
	TCHAR	szDisplayName[256];			//	网络适配器的显示名称
} NETWORK_ADPT_INFO, *PNETWORK_ADPT_INFO;

//
// 定义本地网络的所有网络适配器的名称信息相关结构，最多支持4个本地网络
//
typedef struct _NETWORK_ADPTS_NAME 
{
	TCHAR	szAdapterName[MAXNUMOFADPT][64];	//	所有本地网络适配器的名称数组
	DWORD   NumOfAdapters;						//	实际本地网络的数目
} NETWORK_ADPTS_NAME, *PNETWORK_ADPTS_NAME;

///////////////////////////////////////////////////////////////////////////////////////
// 功能描述：
//    将IP地址字符串形式转化为DWORD值，在NETWORK_ADPT_INFO中需要代入IP地址的DWORD值。
//           
//
// 输入参数 
//    IPAddressString:  IP地址的字符串，如: _T("192.168.201.182")
//
// 输出参数
//    IPAddressValue:  IP地址的DWORD值。
//
// 返回值 = TRUE:	操作成功
//        = FALSE:	操作失败
///////////////////////////////////////////////////////////////////////////////////////
BOOL  StringToIPAddr(TCHAR *IPAddressString, DWORD *IPAddressValue);

///////////////////////////////////////////////////////////////////////////////////////
// 功能描述：
//    获取本地网络的个数以及相应的各个网络适配器的名称。
//           
// 输出参数
//    pAdptsName:  获取得到的NETWORK_ADPTS_NAME结构参数。
//
// 返回值 = TRUE:	操作成功
//        = FALSE:	操作失败
///////////////////////////////////////////////////////////////////////////////////////
BOOL  GetNetWorkAdaptersName( PNETWORK_ADPTS_NAME pAdptsName );

///////////////////////////////////////////////////////////////////////////////////////
// 功能描述：
//    获取本地网络适配器的相关参数，包括IP、子网掩码、网关等。
//           
// 输出参数
//    pAdptInfo:  获取得到的NETWORK_ADPT_INFO结构参数。
//
// 返回值 = TRUE:	操作成功
//        = FALSE:	操作失败
///////////////////////////////////////////////////////////////////////////////////////
BOOL  GetNetWorkAdapterInfo( LPTSTR szAdapterName, PNETWORK_ADPT_INFO pAdptInfo );

///////////////////////////////////////////////////////////////////////////////////////
// 功能描述：
//    设置本地网络适配器的相关参数，包括IP、子网掩码、网关等。
//           
// 输入参数
//    pAdptInfo:  需要设置的NETWORK_ADPT_INFO结构参数。
//
// 返回值 = TRUE:	操作成功
//        = FALSE:	操作失败
///////////////////////////////////////////////////////////////////////////////////////
BOOL  SetNetWorkAdapterInfo( LPTSTR szAdapterName, PNETWORK_ADPT_INFO pAdptInfo );

#endif