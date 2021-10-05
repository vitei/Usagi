/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma once

#ifndef SOCK_GAME_CONNECTION_H
#define SOCK_GAME_CONNECTION_H

#include <Engine/Network/NetGameConnection.h>
#include "Engine/Network/NetAvailableGames.h"
#include "Engine/Core/stl/list.h"
#include <Engine/Memory/FastPool.h>

#define NET_GAME_INVALID_GAME_ID (-1)

namespace usg
{

class SockGameConnection : public NetGameConnection
{
public:
	SockGameConnection();
	virtual ~SockGameConnection();

	static SockGameConnection* Inst() { return m_sockInst; }

	// Interface functions
	virtual void AttemptToJoinGame(NetAvailableGame* game);
	virtual void ConnectToGame(uint32 uGameId) { }
	virtual void Update(float deltaTime);	
	virtual void HostGame();
	virtual void LeaveGame();

	virtual	NetAvailableGame* GetAvailableGame(uint32 idx);
	virtual uint32 GetNumAvailableGames();
	virtual void ForceDisconnect();
	virtual bool IsDisconnected() {
		return m_gameID == NET_GAME_INVALID_GAME_ID
			&& m_attemptingToConnectGameID == NET_GAME_INVALID_GAME_ID;
	}
	virtual bool IsInErrorState() { return false; }

	// Internal
	void MigrateToLobby(sint64 newGameUID, bool isHost);	
	void UpdateLookingForClients(float deltaTime);
	void OnClientWantsToJoin(struct NetMessageJoinGame* join, NetPacket* inPacket);
	void OnGameAvailable(struct NetMessageGameAvailable* game, NetPacket* inPacket);
	sint64 GetGameID() { return m_gameID; }
	bool IsMigrationValid() { return m_migrationTime > 0.0f; }
	sint64 GetOldMigrationID() { return m_migrationOldGameID; }


private:
	static SockGameConnection* m_sockInst;

	void RemoveStaleGames();
	void ClearGames();
	void AddOrUpdateGame(NetMessageGameAvailable* message);

	float m_migrationTime;
	sint64 m_migrationOldGameID;
	sint64 m_gameID;

	bool m_isDisconnecting;
	float m_handshakeTimer;
	sint64 m_attemptingToConnectGameID;
	float m_attemptingToDisconnectTimer;
	list<NetAvailableGame*> m_availableGames;
	FastPool<NetAvailableGame> m_gamePool;

	friend class SockMessageHandler;
};

}

#endif // SOCK_GAME_CONNECTION_H
