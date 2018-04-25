/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_PLATFORM_WIN_H
#define NET_PLATFORM_WIN_H

// If windows...
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

typedef SOCKADDR_IN SocketIn;
typedef in_addr SocketInAddr;
typedef ip_mreq SocketMreq;

#endif
