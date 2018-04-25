/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef SOCK_MULTI_LINK_H
#define SOCK_MULTI_LINK_H

#include <Engine/Network/NetPlatform.h>
#include <Engine/Network/NetPacket.h>

namespace usg
{

// LAN Multicast transportation layer (Windows)
class SockMultiLink
{
public:
	SockMultiLink() { }
	~SockMultiLink() { }

	bool Initialize();
	bool Shutdown();

	bool Send(NetPacket* outPacket);
	bool Recv(NetPacket* inPacket);
private:
	bool InitializeClient();
	bool InitializeServer();

	bool m_isInitialized;

	// Sender data
	SOCKET m_clientSocket;
	int m_clientSockLength;
	SocketIn m_clientSaddr;
	SocketInAddr m_clientInAddr;

	// Listener data
	SOCKET m_serverSocket;
	int m_serverSockLength;
	SocketIn m_serverSaddr;
	SocketMreq m_serverImReq;

};

}

#endif
