/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_PLATFORM_OSX_H
#define NET_PLATFORM_OSX_H


#include "Engine/Common/Common.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef sockaddr_in SocketIn;
typedef in_addr_t SocketInAddr;
typedef ip_mreq SocketMreq;

#define SOCKET_ERROR (-1)

#endif
