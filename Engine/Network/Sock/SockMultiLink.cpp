/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "SockMultiLink.h"
#include <string.h>

namespace usg
{


bool SockMultiLink::Initialize()
{
	// Initialize the client portion
	m_isInitialized = InitializeClient();

	// If the client was ok, set up the server
	if (m_isInitialized)
	{
		m_isInitialized = InitializeServer();
	}
	return m_isInitialized;
}

// Shut down
bool SockMultiLink::Shutdown()
{
	if (m_isInitialized == false)
		return false;

#ifdef PLATFORM_PC
	closesocket(m_clientSocket);
	shutdown(m_clientSocket, 2);

	closesocket(m_serverSocket);
	shutdown(m_serverSocket, 2);

#elif defined(PLATFORM_OSX)
	close(m_clientSocket);
	shutdown(m_clientSocket, 2);
	
	close(m_serverSocket);
	shutdown(m_serverSocket, 2);

#endif

	m_isInitialized = false;
	return true;

}


// Set up the client for sending
bool SockMultiLink::InitializeClient()
{
	// Zero out everything
	memset(&m_clientSaddr, 0, sizeof(SocketIn));
	memset(&m_clientInAddr, 0, sizeof(SocketInAddr));

	static char ttl = MULTICAST_TTL;

	// Set up the socket
	m_clientSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (m_clientSocket < 0) { return false; }

	// Send data info
	m_clientSaddr.sin_family = PF_INET;
	m_clientSaddr.sin_port = htons(0);
	m_clientSaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int status = bind(m_clientSocket, (sockaddr*)&m_clientSaddr, sizeof(sockaddr_in));

	// Binding failed
	if (status < 0)
	{
		Shutdown();
		return false;
	}

	// Set options
#if defined(PLATFORM_PC)
	m_clientInAddr.s_addr = INADDR_ANY;
#else
	m_clientInAddr = INADDR_ANY;
#endif

	static char one = 1;
	setsockopt(m_clientSocket, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&m_clientInAddr, sizeof(in_addr));
	setsockopt(m_clientSocket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(char));
	setsockopt(m_clientSocket, IPPROTO_IP, IP_MULTICAST_LOOP, &one, sizeof(char));

	// Set multicast addresses
	m_clientSaddr.sin_family = PF_INET;
	ASSERT(false);
	//m_clientSaddr.sin_addr.s_addr = inet_addr(MULTICAST_IP_ADDRESS);
	m_clientSaddr.sin_port = htons(MULTICAST_PORT);

	m_clientSockLength = sizeof(sockaddr_in);

	// Ok
	return true;
}

// initialize server
bool SockMultiLink::InitializeServer()
{
	// Zero out the server structs
	memset(&m_serverSaddr, 0, sizeof(SocketIn));
	memset(&m_serverImReq, 0, sizeof(SocketMreq));

	// Open up the udp sockets
	m_serverSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (m_serverSocket < 0) { return false; }

	// Setup the recv information
	m_serverSaddr.sin_family = PF_INET;
	m_serverSaddr.sin_port = htons(MULTICAST_PORT);
	m_serverSaddr.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef PLATFORM_OSX
	m_serverSaddr.sin_len = sizeof(sockaddr_in);
	m_serverSaddr.sin_addr.s_addr = inet_addr(MULTICAST_IP_ADDRESS);
#endif

	// Bind the socket
	int status = bind(m_serverSocket, (sockaddr*)&m_serverSaddr, sizeof(sockaddr));
	if (status < 0)
	{
		Shutdown();
		return false;
	}

	// set up interface
	ASSERT(false);
	//m_serverImReq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP_ADDRESS);
	m_serverImReq.imr_interface.s_addr = INADDR_ANY;
	setsockopt(m_serverSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&m_serverImReq, sizeof(ip_mreq));
	m_serverSockLength = sizeof(sockaddr_in);
	
	static char unused = 0;
	
	setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&unused, sizeof(unused));

#ifdef PLATFORM_OSX
  setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEPORT, (char*)&unused, sizeof(unused));
#endif

	// everything set up properly
	return true;
}

// Send on the multicast connection
bool SockMultiLink::Send(NetPacket* packetOut)
{
	// Make sure we have a connection
	if (m_isInitialized == false) { return false; }

	int dataLength = packetOut->header.dataLength + sizeof(NetPacketHeader);

	// Send it
	sendto(m_clientSocket, (const char*)packetOut, dataLength, 0, (sockaddr*)&m_clientSaddr, (int)m_clientSockLength);

	return true;
}


// Receive data on the multicast
bool SockMultiLink::Recv(NetPacket* inPacket)
{
	// Make sure a connection exists
	if (m_isInitialized == false) { return false; }
	int bytesread = 0;

	// Setup the FD
	fd_set m_serverWfds;
	FD_ZERO(&m_serverWfds);
	FD_SET(m_serverSocket, &m_serverWfds);

	// Initialize timer
	timeval m_serverTV;
	m_serverTV.tv_sec = 0;
	m_serverTV.tv_usec = NET_TV_TIMEOUT;

	// Select
	int status = select((int)m_serverSocket, &m_serverWfds, 0, 0, &m_serverTV);
	if (status == SOCKET_ERROR)
	{
		// Something went wrong
		Shutdown();
		return false;
	}

	// Nothing to read
	if (status == 0) { return false; }

#ifdef PLATFORM_PC
	// Receive the data
		bytesread = recvfrom(m_serverSocket, (char*)inPacket, NETWORK_PACKET_MAX_LENGTH, 0, (sockaddr*)&m_serverSaddr, 	&m_serverSockLength);
#else
	unsigned int val = (unsigned int)m_serverSockLength;
	bytesread = recvfrom(m_serverSocket, (char*)inPacket, NETWORK_PACKET_MAX_LENGTH, 0, (sockaddr*)&m_serverSaddr, 	&val);
#endif
	// reading failed, we need to stop reading
	if (bytesread == -1)
	{
		Shutdown();
		return false;
	}

	// Configure read packet info
#ifdef PLATFORM_PC
	inPacket->IP = m_serverSaddr.sin_addr.S_un.S_addr;
#else
	inPacket->IP = m_serverSaddr.sin_addr.s_addr;
#endif

	inPacket->port = m_serverSaddr.sin_port;


	inPacket->length = bytesread;
	return true;
}

}