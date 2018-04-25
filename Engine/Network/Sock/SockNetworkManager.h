/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef SOCK_NETWORK_MANAGER_H
#define SOCK_NETWORK_MANAGER_H

#include <Engine/Network/NetManager.h>

#include "SockClient.h"
#include "SockClientLink.h"
#include "SockMultiLink.h"
#include "SockGameConnection.h"
#include "SockMessageHandler.h"

namespace usg
{

	struct InitLocalHostRequest;
	struct InitLocalClientRequest;

// Network class manager
class SockNetworkManager : public NetManager
{
public:
	SockNetworkManager(UsagiInetCore* pInetCore);
	virtual ~SockNetworkManager();

	// Get instance specific to Sock network
	static SockNetworkManager* Inst() { return m_sockInst;  }

	void InitLocalHost(const InitLocalHostRequest& req) {}
	void InitLocalClient(const InitLocalClientRequest& req) {}
	void InitInetAutoMatch(const InitInetAutoMatchRequest& req) {}
	virtual void CloseSession() { ASSERT(false); }

	// Local host
	virtual SessionOpenForParticipationStatus GetOpenForParticipationStatus() const { ASSERT(false && "Sock Network game not currently supported."); return SESSION_OPEN; };
	virtual bool SetOpenForParticipation(bool bOpen) { ASSERT(false && "Sock Network game not currently supported."); return false; }

	// virtual
	virtual void Initialize(const void* pAppData = NULL, uint32 uAppDataSize = 0);
	virtual void Update(float deltaTime);
	virtual void DisconnectAll(bool bGameOver = false);
	virtual void StartHosting();
	virtual void StartClient();
	
	// Send data out
	void SendToClient(long long uid, NetMessage& message, bool reliable);
	virtual void SendToMulti(NetMessage& message);

	// Game-related
	virtual void AttemptToJoinGame(NetAvailableGame* game);
	uint32 GetNumAvailableGames() { return m_gameConnection.GetNumAvailableGames(); }
	struct NetAvailableGame* GetAvailableGame(uint32 idx) { return m_gameConnection.GetAvailableGame(idx); }
	virtual NetGameConnection* GetGameConnection() { return &m_gameConnection;  }
	sint64 GetGameID(){ return m_gameConnection.GetGameID(); }

	// Client related
	virtual void SetHost(sint64 hostID);
	SockClient* AddClient(NetSimplePlayerData* clientData);
	virtual bool RemoveClient(class NetClient* client, bool migrate, bool bGameOver = false);
	virtual NetClient* GetClient(int idx){ return &m_gameClients[idx]; }
	virtual const NetClient* GetClient(int idx) const { return &m_gameClients[idx]; }
	
	// Get the local player's name
	void FillName(char* nameOut);

private:
	sint64 m_hostID;
	static SockNetworkManager* m_sockInst;

	// Update
	virtual void UpdateHostingLobby(float deltaTime);
	virtual void UpdateClientLobby(float deltaTime);
	virtual void UpdateJoiningLobby(float deltaTime);

	// Client/packet related
	virtual class SockClient* GetClientFromPacket(NetPacket* inPacket);
	virtual void SendOnClientLink(NetPacket* outPacket);
	void SendOnMultiLink(NetPacket* outPacket);

	// Host lists
	void SyncHostList(NetMessageHostList* hostList);
	sint64 m_nextHostList[USAGI_NET_MAX_CLIENTS - 1];
	double m_hostListSyncTime;
	float m_hostListNextSend;

	// Handles available game, joining games
	SockGameConnection m_gameConnection;

	// Handle incoming messages
	SockMessageHandler m_messageHandler;
	
	// Lists of connected clients
	SockClient m_multicastClient;
	SockClient m_gameClients[USAGI_NET_MAX_CLIENTS];

	// Links to network
	SockClientLink m_clientLink;
	SockMultiLink m_multiLink;

	friend class SockGameConnection;
	friend class SockMessageHandler;
	friend class SockClient;

};

}

#endif
