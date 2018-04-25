/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "SockClientLink.h"
#include "Engine/Network/NetPacket.h"
#include <string.h>

namespace usg
{

#include OS_HEADER(Engine/Network, NetPlatform_ps.h)

bool SockClientLink::Initialize()
{
	// Initialize sending
	m_isInitialized = InitializeSend();
	if (m_isInitialized)
	{
		// Initialize receiving
		m_isInitialized = InitializeReceive();
	}
	return m_isInitialized;
}

bool SockClientLink::Shutdown()
{
	if (m_isInitialized == false)
		return false;

#ifdef PLATFORM_PC
	// Close the client link
	closesocket(m_clientSocket);
	shutdown(m_clientSocket, 2);

	// Close the server
	closesocket(m_serverSocket);
	shutdown(m_serverSocket, 2);
#elif defined(PLATFORM_OSX)
	// Close the client link
	close(m_clientSocket);
	shutdown(m_clientSocket, 2);

	// Close the server
	close(m_serverSocket);
	shutdown(m_serverSocket, 2);
#endif

	return true;
}

bool SockClientLink::InitializeSend()
{
	// Clear out the send struct
	memset(&m_clientSaddr, 0, sizeof(SocketIn));
	m_clientSocket = -1;

#if defined(PLATFORM_PC) || defined(PLATFORM_OSX)

	// Allocate the socket
	m_clientSocket = socket(PF_INET, SOCK_DGRAM, 0);
	
	// Set the family to the internet
	m_clientSaddr.sin_family = PF_INET;

#endif	

	if (m_clientSocket == -1) { return false; }

	// good
	return true;
}

// Init Receiving socket
bool SockClientLink::InitializeReceive()
{
	// Zero out the send/recv structs
	memset(&m_serverSaddr, 0, sizeof(SocketIn));
	memset(&m_serverInAddr, 0, sizeof(SocketIn));

#if defined(PLATFORM_PC) || defined(PLATFORM_OSX)
	// Create the socket
	m_serverSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (m_serverSocket == -1) { return false; }

	// Set up the local bindings
	m_serverSaddr.sin_family = PF_INET;
	m_serverSaddr.sin_port = htons(CLIENT_LINK_LOCAL_PORT);
	m_serverSaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind the socket
	int ret = bind(m_serverSocket, (sockaddr*)&m_serverSaddr, sizeof(m_serverSaddr));
	if (ret == SOCKET_ERROR)
	{
		Shutdown();
		return false;
	}
	return true;

#else
	return false;
#endif

}

// Send packet
bool SockClientLink::Send(NetPacket* packetOut)
{
	// Validate the connection
	if (m_isInitialized == false) { return false; }
	
	int ret = -1;
	int dataLength = packetOut->header.dataLength + sizeof(NetPacketHeader);

	// Setup sending data
	m_clientSaddr.sin_port = htons(CLIENT_LINK_LOCAL_PORT);
	m_clientSaddr.sin_addr.s_addr = (packetOut->IP);

	// Send the data
	ret = sendto(m_clientSocket, (const char*)packetOut, dataLength, 0, (sockaddr*)&m_clientSaddr, sizeof(m_clientSaddr));

	// Return was a non-error code
	return ret != -1;
}

// Receive
bool SockClientLink::Recv(NetPacket* inPacket)
{
	// Validate the connection
	if (m_isInitialized == false) { return false; }

	int ret = 0;
#if defined(PLATFORM_PC) || defined(PLATFORM_OSX)

	// Test connection
	fd_set m_serverWdfs;
	FD_ZERO(&m_serverWdfs);
	FD_SET(m_serverSocket, &m_serverWdfs);
	
	// Setup the timeout
	timeval m_serverTV;
	m_serverTV.tv_sec = 0;
	m_serverTV.tv_usec = NET_TV_TIMEOUT;
	
	// Select
	ret = select((int)(1 + m_serverSocket), &m_serverWdfs, 0, 0, &m_serverTV);
	
	// No data
	if (ret == 0) return false;

	// If failed to select, quit
	if (ret == SOCKET_ERROR)
	{
		Shutdown();
		return false;
	}

#endif

#ifdef PLATFORM_PC

	// Data was received and needs to be read in
	int senderSize = sizeof(SOCKADDR_IN);
	ret = recvfrom(m_serverSocket, (char*)inPacket, NETWORK_PACKET_MAX_LENGTH, 0, (SOCKADDR*)&m_serverInAddr, &senderSize);
	
	// Set the packet information
	inPacket->IP = m_serverInAddr.sin_addr.S_un.S_addr;
	inPacket->port = m_serverInAddr.sin_port;

#elif defined(PLATFORM_OSX)

	// Data was rec'd 
	unsigned int senderSize = sizeof(sockaddr_in);
	ret = recvfrom(m_serverSocket, (char*)inPacket, NETWORK_PACKET_MAX_LENGTH, 0, (sockaddr*)&m_serverInAddr, &senderSize);

	// Set the packet info
	inPacket->IP = m_serverInAddr.sin_addr.s_addr;
	inPacket->port = m_serverInAddr.sin_port;

#else
	ret = 0;
#endif

	// Something bad happened
	if (ret == -1)
	{
		Shutdown();
		return false;
	}

	// Success
	inPacket->length = ret;
	return true;
}

}