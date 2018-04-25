/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef SOCK_CLIENT_H
#define SOCK_CLIENT_H

#include <Engine/Network/NetClient.h>

namespace usg
{

// Data that holds all information to send and receive from an endpoint
class SockClient : public NetClient
{
public:
	SockClient();
	virtual ~SockClient();

	// Main functions
	virtual void Disconnect();

	// Identification of new client
	void SetPlayerData(NetSimplePlayerData* clientData, int playerID);
	
	// Getters
	unsigned int GetIP() { return m_ipAddress; }
	unsigned short GetPort() { return m_port; }
	virtual sint64 GetUID() { return m_UID; }


	// Utilities
	void MarkNeedsToMigrate(bool hasnt) { m_needsToMigrate = hasnt; m_requestedUsAsHost = false; }
	bool GetRequestedUsAsHost() { return m_requestedUsAsHost; }
	
protected:
	// Internal utilities
	virtual bool CheckForTimeout();
	virtual void CheckIncomingPacketForMigration(NetPacket* inPacket);
	virtual void SetOutgoingPacketHeader(NetPacket* packet);
	virtual void ReadMessage(NetMessage* message);
	virtual void SendPacketToMulti();

private:

	// Migration data
	bool m_needsToMigrate;
	bool m_requestedUsAsHost;

	// Identification
	long long m_UID;
	unsigned int m_ipAddress;
	unsigned short m_port;

};

}

#endif
