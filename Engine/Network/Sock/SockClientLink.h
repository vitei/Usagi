/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef SOCK_CLIENT_LINK_H
#define SOCK_CLIENT_LINK_H

#include <Engine/Network/NetClientLink.h>
#include <Engine/Network/NetPacket.h>

namespace usg
{

// Transportation layer for clients
class SockClientLink : public NetClientLink
{
public:
	SockClientLink() : NetClientLink() { }
	virtual ~SockClientLink() { }

	virtual bool Initialize();
	virtual bool Shutdown();

	virtual bool Send(struct NetPacket* outPacket);
	virtual bool Recv(struct NetPacket* inPacket);

private:
	bool InitializeSend();
	bool InitializeReceive();

	// Client-specific information
	SOCKET m_clientSocket;
	SocketIn m_clientSaddr;

	// Listener information
	SOCKET m_serverSocket;
	SocketIn m_serverSaddr;
	SocketIn m_serverInAddr;

};

}

#endif
