/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Network/NetPlatform.h"

#include "Engine/Network/NetAvailableGames.h"
#include "../UsagiNetwork.h"
#include "SockGameConnection.h"
#include "SockNetworkManager.h"


#define USAGI_NET_CLIENT_CONNECTION_TIME (1.0f)
#define USAGI_NET_DISCONNECT_TIME (2.0f)
#define USAGI_NET_HOST_LOBBY_SEND_TIME (1.0f)
#define USAGI_STALE_GAME_TIME (5)
#define USAGI_NET_MIGRATION_TIME (4.0f)

namespace usg {

SockGameConnection* SockGameConnection::m_sockInst = 0;

SockGameConnection::SockGameConnection()
	: NetGameConnection()
	, m_migrationTime(0)
	, m_migrationOldGameID(NET_GAME_INVALID_GAME_ID)
	, m_gameID(NET_GAME_INVALID_GAME_ID)
	, m_isDisconnecting(false)
	, m_attemptingToConnectGameID(NET_GAME_INVALID_GAME_ID)
	, m_gamePool(16)
{
	m_sockInst = this;
}

SockGameConnection::~SockGameConnection()
{
	ForceDisconnect();
}

void SockGameConnection::ForceDisconnect()
{
	m_gameID = NET_GAME_INVALID_GAME_ID;
	m_attemptingToConnectGameID = NET_GAME_INVALID_GAME_ID;
	m_attemptingToDisconnectTimer = 0;
	m_isDisconnecting = false;
	m_handshakeTimer = 0.0f;

	ClearGames();
}

void SockGameConnection::ClearGames()
{
	for(NetAvailableGame* game : m_availableGames)
	{
		m_gamePool.Free(game);
	}
	m_availableGames.clear();
}

void SockGameConnection::HostGame()
{
	// Set the lobby identity (arbitrary random value)
	m_attemptingToConnectGameID = NET_GAME_INVALID_GAME_ID;
	m_gameID = SockNetworkManager::Inst()->GenerateGameUID();
	m_handshakeTimer = 0.0f;
}

// We migrated to a new lobby with a different ID
void SockGameConnection::MigrateToLobby(sint64 newGameUID, bool isHost)
{
	if (isHost)
	{
		m_migrationTime = USAGI_NET_MIGRATION_TIME;
		m_migrationOldGameID = m_gameID;
		m_gameID = newGameUID;
	}
	else
	{
		m_gameID = newGameUID;
	}
}

void SockGameConnection::AttemptToJoinGame(NetAvailableGame* game)
{
	m_attemptingToConnectGameID = game->gameUID;
}

void SockGameConnection::LeaveGame()
{
	// We are trying to leave this lobby
	NetMessageLeaveGame nmlg;
	nmlg.gameUID = m_gameID;
	SockNetworkManager::Inst()->SendToAll(nmlg, false);

	m_isDisconnecting = true;
	m_attemptingToDisconnectTimer = USAGI_NET_DISCONNECT_TIME;
}

uint32 SockGameConnection::GetNumAvailableGames()
{
	return (uint32)m_availableGames.size();
}

// See if there is a better way to do this retrieval
NetAvailableGame* SockGameConnection::GetAvailableGame(uint32 idx)
{
	list<NetAvailableGame*>::iterator it = m_availableGames.begin();
	while (idx > 0)
	{
		++it;
		idx--;
	}

	return (*it);
}


void SockGameConnection::UpdateLookingForClients(float deltaTime)
{
	// See if we need to sound out the next message
	m_handshakeTimer -= deltaTime;
	if (m_handshakeTimer > 0.0f)
		return;
	m_handshakeTimer = USAGI_NET_HOST_LOBBY_SEND_TIME;

	// Send out a game available message
	NetMessageGameAvailable game;
	game.gameUID = m_gameID;
	game.gameIsAvailable = true;

	// Set the host information
	game.client[0].active = true;
	game.client[0].uid = NetManager::Inst()->GetUID();
	game.client[0].ip = 0;
	game.client[0].port = 0;
	SockNetworkManager::Inst()->FillName(game.client[0].name);
	
	// Fill the clients
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS - 1; i++)
	{
		SockClient* client = (SockClient*)SockNetworkManager::Inst()->GetClient(i);

		game.client[i + 1].active = 0;
		if (client->GetIsActive())
		{
			game.client[i + 1].active =	1;
			game.client[i + 1].color =	client->GetColor();
			game.client[i + 1].ip =		client->GetIP();
			game.client[i + 1].port =	client->GetPort();
			game.client[i + 1].uid =	client->GetUID();

			// Fill the name
			memcpy(game.client[i + 1].name, client->GetName(), USAGI_NET_CLIENT_MAX_NAME_LENGTH);
		}
	}


	SockNetworkManager::Inst()->SendToMulti(game);
}

void SockGameConnection::Update(float deltaTime)
{
	// Remove old games
	RemoveStaleGames();

	// In a game, just return
	if (m_gameID != NET_GAME_INVALID_GAME_ID)
		return;

	// Update migration time
	if (m_migrationTime > 0)
	{
		m_migrationTime -= deltaTime;
		m_migrationOldGameID = NET_GAME_INVALID_GAME_ID;
	}

	// Not trying to join, just return
	if (m_attemptingToConnectGameID == NET_GAME_INVALID_GAME_ID)
		return;

	// Trying to connect...
	m_handshakeTimer -= deltaTime;
	if (m_handshakeTimer < 0.0f)
	{
		m_handshakeTimer = USAGI_NET_CLIENT_CONNECTION_TIME;

		// Send a message on multicast to join this game
		NetMessageJoinGame joinGame;
		joinGame.gameUID = m_attemptingToConnectGameID;
		SockNetworkManager::Inst()->FillName(joinGame.userName);

		// Send out
		SockNetworkManager::Inst()->SendToMulti(joinGame);
	}
}

void SockGameConnection::OnClientWantsToJoin(NetMessageJoinGame* join, NetPacket* inPacket)
{
	// Throw away packet if we aren't in a game
	if (m_gameID == NET_GAME_INVALID_GAME_ID)
		return;

	// Not our lobby
	if (m_gameID != join->gameUID)
		return;

	DEBUG_PRINT("SockGameConnection : ClientWantsToJoin (%s)\n", join->userName);

	// Tell lobby that we have added a new client
	NetMessageNewClient nmc;
	nmc.playerData.active = true;
	nmc.playerData.color = join->color;
	nmc.playerData.ip = inPacket->IP;
	nmc.playerData.port = inPacket->port;
	nmc.playerData.uid = inPacket->header.clientUID;

	for (int i = 0; i < USAGI_NET_CLIENT_MAX_NAME_LENGTH; i++)
	{
		nmc.playerData.name[i] = join->userName[i];
	}
	nmc.gameUID = m_gameID;
	SockNetworkManager::Inst()->SendToAll(nmc, true);

	// Add the client

	SockNetworkManager::Inst()->AddClient(&nmc.playerData);
	m_handshakeTimer = 0;
}

void SockGameConnection::OnGameAvailable(NetMessageGameAvailable* game, NetPacket* in)
{
	// If we are in a game, throw it away
	if (m_gameID != NET_GAME_INVALID_GAME_ID && m_gameID != game->gameUID)
		return;


	// Update the host's ip/port
	game->client[0].ip = in->IP;
	game->client[0].port = in->port;

	// Update the game list with this game (may even be the one we are trying to join)
	AddOrUpdateGame(game);

	// If it's not the game we care about, quit
	if (m_attemptingToConnectGameID == game->gameUID)
	{
		// If it's unavailable, report that and then quit
		if (game->gameIsAvailable == 0)
		{
			// Report that the game was unavailable
			UsagiNet::OnJoinGameUnavailable();

			// stop trying to connect
			m_attemptingToConnectGameID = NET_GAME_INVALID_GAME_ID;
			return;
		}

		// See if we are on the list of clients
		for (int i = 1; i < USAGI_NET_MAX_CLIENTS; i++)
		{
			if (game->client[i].uid == SockNetworkManager::Inst()->GetUID())
			{
				// Add clients
				for (int j = 0; j < USAGI_NET_MAX_CLIENTS; j++)
				{
					// Don't add local player
					if (i == j) continue;

					// Don't add inactive players
					if (game->client[j].active == 0) continue;

					SockNetworkManager::Inst()->AddClient(&game->client[j]);
				}

				// Set additional game data
				SockNetworkManager::Inst()->SetHost(game->client[0].uid);
				m_gameID = game->gameUID;
				m_attemptingToConnectGameID = NET_GAME_INVALID_GAME_ID;
				SockNetworkManager::Inst()->StartClient();
				break;
			}
		}
	}
}

void SockGameConnection::AddOrUpdateGame(NetMessageGameAvailable* message)
{
	// See if this game already exists
	NetAvailableGame* game = 0;
	list<NetAvailableGame*>::iterator it = m_availableGames.begin();
	while (it != m_availableGames.end())
	{
		NetAvailableGame* temp = (*it);
		if (temp->gameUID == message->gameUID)
		{
			temp->UpdateData(message);
			return;
		}
		++it;
	}

	// If the game is unavailable, just return
	if (message->gameIsAvailable == 0)
		return;

	// No match, add a new game to the list
	game = m_gamePool.Alloc();
	game->UpdateData(message);
	m_availableGames.push_back(game);

}

void SockGameConnection::RemoveStaleGames()
{
	double currentTime = net_get_time();

	list<NetAvailableGame*>::iterator it = m_availableGames.begin();
	while (it != m_availableGames.end())
	{
		NetAvailableGame* game = (*it);
		++it;
		if (currentTime - game->discoveredTime > USAGI_STALE_GAME_TIME)
		{
			if (m_attemptingToConnectGameID == game->gameUID)
			{
				SockNetworkManager::Inst()->DisconnectAll();
				m_attemptingToConnectGameID = -1;

				// DisconnectAll frees all games in this list, so return to avoid a crash
				return;
			}

			// Free and remove it
			m_gamePool.Free(game);
			m_availableGames.remove(game);
		}
	}
}

}
