/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "NetPlatform_ps.h"
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")

namespace usg {

static WSAData WSAStartupData;

void net_startup()
{
	// Startup Winsock
	WSAStartup(MAKEWORD(2, 2), &WSAStartupData);
}

void net_shutdown()
{
	// Shut it down
	WSACleanup();
}

long long net_get_uid()
{
	IP_ADAPTER_INFO AdapterInfo[16];
	DWORD dwBufLen = sizeof(AdapterInfo);
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus != ERROR_SUCCESS)
	{
		return -1;
	}
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	return *((long long*)pAdapterInfo->Address);
}

}
