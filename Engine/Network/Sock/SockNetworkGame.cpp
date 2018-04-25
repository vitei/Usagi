/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Network/NetPlatform.h"
#include "Engine/Network/NetAvailableGames.h"
#include "SockNetworkGame.h"
#include "Engine/Network/Sock/SockNetworkManager.h"

#define SOCK_LOOKING_FOR_GAME_TIME (2)

using namespace usg;

SockNetworkGame::SockNetworkGame()
	: m_state(LOOKING_FOR_GAME)
	, m_stateTimer(0.f)
	, m_tryingToConnectID(0)
{
}

bool SockNetworkGame::AttemptConnection(float deltaTime)
{
	switch(m_state)
	{
	case LOOKING_FOR_GAME:  return UpdateLookingForGame(deltaTime);
	case TRYING_TO_CONNECT: return UpdateTryingToConnect(deltaTime);
	case CONNECTED:         return true;
	}

	ASSERT(0);
	return false;
}

bool SockNetworkGame::CheckConnection()
{
	return NetManager::Inst()->IsInitialized();
}

bool SockNetworkGame::UpdateLookingForGame(float deltaTime)
{
	// Poll the network manager for any games that exist
	if (SockNetworkManager::Inst()->GetNumAvailableGames() > 0)
	{
		// Try to join the first game on this list
		NetAvailableGame* game = SockNetworkManager::Inst()->GetAvailableGame(0);
		m_tryingToConnectID = game->gameUID;
		m_stateTimer = SOCK_LOOKING_FOR_GAME_TIME;
		m_state = TRYING_TO_CONNECT;

		NetManager::Inst()->AttemptToJoinGame(game);
	}
	else
	{
		m_stateTimer -= deltaTime;
		if (m_stateTimer <= 0)
		{
			CreateNewLobby();
		}
	}

	return m_state == CONNECTED;
}

bool SockNetworkGame::UpdateTryingToConnect(float deltaTime)
{
	if (SockNetworkManager::Inst()->GetGameID() == NET_GAME_INVALID_GAME_ID)
	{
		m_stateTimer -= deltaTime;
		if (m_stateTimer <= 0)
		{
			m_state = LOOKING_FOR_GAME;
			m_stateTimer = SOCK_LOOKING_FOR_GAME_TIME;
		}
	}
	else if (SockNetworkManager::Inst()->GetGameID() == m_tryingToConnectID)
	{
		// We connected. Sync.
		m_state = CONNECTED;
	}

	return m_state == CONNECTED;
}

void SockNetworkGame::UpdateConnected()
{
	if(m_state != CONNECTED && CheckConnection() == true)
	{
		m_state = CONNECTED;
	}

	// TODO Host migration
	/*
	// Switch to make a client the host if he accepted host migration
	if (m_state == PLAYING_AS_CLIENT)
	{
		if (NetManager::Inst()->IsHost())
		{
			m_state = PLAYING_AS_HOST;
		}
		else if (NetManager::Inst()->GetNumConnectedClients() == 0)
		{
			NetManager::Inst()->DisconnectAll();
			m_state = LOOKING_FOR_GAME;
			m_stateTimer = SOCK_LOOKING_FOR_GAME_TIME;
			return;
		}
		else if (NetManager::Inst()->GetHostUID() == NET_CLIENT_INVALID_ID)
		{
			//NetManager::Inst()->MigrateHost();
		}
	}
	*/
}

void SockNetworkGame::CreateNewLobby()
{
	m_state = CONNECTED;
	NetManager::Inst()->DisconnectAll();
	NetManager::Inst()->StartHosting();
}
