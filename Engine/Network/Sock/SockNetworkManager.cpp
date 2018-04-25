/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Network/NetPlatform.h"
#include "SockNetworkManager.h"
#include <Engine/Network/NetDataCompressor.h>
#include "Engine/Network/NetworkGame.h"
#include "../UsagiNetwork.h"

namespace usg {

static const float NET_HOST_LIST_NEXT_SEND_TIME = 2.0f;

SockNetworkManager* SockNetworkManager::m_sockInst = 0;

SockNetworkManager::SockNetworkManager(UsagiInetCore* pInetCore)
	: NetManager(pInetCore)
{
	m_sockInst = this;
}

SockNetworkManager::~SockNetworkManager()
{
	DisconnectAll(true);

	m_clientLink.Shutdown();
	m_multiLink.Shutdown();

	net_shutdown();
}
   
void SockNetworkManager::Initialize(const void* pAppData, uint32 uAppDataSize)
{
	net_startup();

	NetManager::Initialise();

	m_isInitialized = m_multiLink.Initialize();
	if (m_isInitialized)
		m_isInitialized = m_clientLink.Initialize();

	m_multicastClient.SetActive(true);
	m_multicastClient.SetPlayerData(0, -1);
}

void SockNetworkManager::DisconnectAll(bool bGameOver)
{
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		RemoveClient(&m_gameClients[i], false, bGameOver);
	}
	m_netState = NS_Disconnected;
	m_gameConnection.ForceDisconnect();
}

void SockNetworkManager::StartHosting()
{
	// This is a lobby
	m_netState = NS_HostingLobby;

	// setup game UID;
	m_gameConnection.HostGame();

	m_NetTime.Reset();
	m_hostListNextSend = NET_HOST_LIST_NEXT_SEND_TIME / 2.0f;
}

void SockNetworkManager::StartClient()
{
	// This is a client
	m_netState = NS_ClientLobby;
	m_NetTime.Reset();
}


void SockNetworkManager::Update(float deltaTime)
{
	// Early out if things didn't resolve
	if (m_isInitialized == false)
		return;

	NetManager::Update(deltaTime);

	// Read any data on the network
	NetPacket inPacket;
	memset(&inPacket, 0, sizeof(NetPacket));

	// Read global messages
	while (m_multiLink.Recv(&inPacket))
	{
		m_messageHandler.HandleIncomingPacket(&inPacket);
		memset(&inPacket, 0, sizeof(NetPacket));
	}

	// Read client messages
	while (m_clientLink.Recv(&inPacket))
	{
		// Handle message
		m_messageHandler.HandleIncomingPacket(&inPacket);
		memset(&inPacket, 0, sizeof(NetPacket));
	}

	// Update global client
	m_multicastClient.Update(deltaTime);

	// Update connected clients
	for (int i = 0; i < 8; i++)
	{
		m_gameClients[i].Update(deltaTime);
	}
}

void SockNetworkManager::AttemptToJoinGame(NetAvailableGame* game)
{
	m_gameConnection.AttemptToJoinGame(game);
	m_netState = NS_JoiningLobby;
}

void SockNetworkManager::UpdateJoiningLobby(float deltaTime)
{
	if (m_gameConnection.GetGameID() != NET_GAME_INVALID_GAME_ID)
		m_netState = NS_ClientLobby;
	if (m_gameConnection.IsDisconnected())
		m_netState = NS_Disconnected;
}

void SockNetworkManager::UpdateClientLobby(float deltaTime)
{
	// Update net time
	m_NetTime.Update(m_host, deltaTime);
	
}

// qsort algorithm to sort new host structs
int SortClientPingPairs(const void* first, const void* second)
{
	HostListPair* f = (HostListPair*)first;
	HostListPair* s = (HostListPair*)second;

	if (f->clientID == NET_CLIENT_INVALID_ID && s->clientID == NET_CLIENT_INVALID_ID)
		return 0;
	if (f->clientID == NET_CLIENT_INVALID_ID && s->clientID != NET_CLIENT_INVALID_ID)
		return 1;
	if (s->clientID == NET_CLIENT_INVALID_ID && f->clientID != NET_CLIENT_INVALID_ID)
		return -1;

	if (f->clientID > s->clientID)
		return 1;
	return -1;

}

void SockNetworkManager::UpdateHostingLobby(float deltaTime)
{
	m_gameConnection.UpdateLookingForClients(deltaTime);

	m_hostListNextSend -= deltaTime;
	if (m_hostListNextSend <= 0.0f)
	{
		m_hostListNextSend = NET_HOST_LIST_NEXT_SEND_TIME;
		NetMessageHostList hostList;
		HostListPair pairs[USAGI_NET_MAX_CLIENTS - 1];

		// Manufacture the host list
		int i = 0;
		for (i = 0; i < USAGI_NET_MAX_CLIENTS - 1; i++)
		{
			pairs[i].clientID = m_gameClients[i].GetUID();
			pairs[i].bestPing = m_gameClients[i].GetPing();
		}
		qsort(pairs, USAGI_NET_MAX_CLIENTS - 1, sizeof(HostListPair), SortClientPingPairs);
		for (i = 0; i < USAGI_NET_MAX_CLIENTS - 1; i++)
		{
			hostList.hostList[i] = pairs[i].clientID;
		}
		hostList.serverTime = m_NetTime.GetServerTimePrecise();
		SendToAll(hostList, true);
	}
}

// Sync a host list received from the host
void SockNetworkManager::SyncHostList(NetMessageHostList* hostList)
{
	m_hostListSyncTime = hostList->serverTime;
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS - 1; i++)
	{
		m_nextHostList[i] = hostList->hostList[i];
	}
}

void SockNetworkManager::SendOnClientLink(NetPacket* outPacket)
{
#ifdef NET_DEBUG_ALLOW_PACKET_INSPECTION
	// These messages were not rec'd
	NetMessage* message = 0;
	int packetOffset = 0;
	int numMessagesToRead = outPacket->header.numMessages;
	while (packetOffset <= outPacket->header.dataLength &&
		numMessagesToRead > 0)
	{
		message = CreateMessageFromBuffer(outPacket->data + packetOffset, false);
		SockNetworkManager::Inst()->DebugMessageTypeSent(message);
		packetOffset += message->GetLength();
		numMessagesToRead--;

		NetManager::Inst()->FreeMessage(message);
	}
#endif
	NetDataCompressorUtil::CompressPacket(outPacket);

	m_clientLink.Send(outPacket);
}

void SockNetworkManager::SendOnMultiLink(NetPacket* outPacket)
{
	NetDataCompressorUtil::CompressPacket(outPacket);

	m_multiLink.Send(outPacket);
}

// Send a message to the multicast
void SockNetworkManager::SendToMulti(NetMessage& message)
{
	m_multicastClient.SendNetMessage(message, false);
}

// Send a message to a specific client
void SockNetworkManager::SendToClient(long long uid, NetMessage& message, bool reliable)
{
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		if (m_gameClients[i].GetIsActive() && m_gameClients[i].GetUID() == uid)
		{
			m_gameClients[i].SendNetMessage(message, reliable);
			return;
		}
	}
}

// Get a client from the data
SockClient* SockNetworkManager::GetClientFromPacket(NetPacket* inPacket)
{
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		// Get the uid from the first message in this packet
		if (m_gameClients[i].GetUID() != inPacket->header.clientUID)
			continue;
		return m_gameClients + i;
	}

	return 0;
}

SockClient* SockNetworkManager::AddClient(NetSimplePlayerData* clientData)
{
	if (clientData->active == 0)
		return 0;
	DEBUG_PRINT("SockNetMan : AddClient (%s)\n", clientData->name);

	// See if this client already has been added
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		if (m_gameClients[i].GetIsActive() == false)
			continue;
		if (m_gameClients[i].GetUID() == clientData->uid)
			return 0;
	}

	// Get the first free client
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		if (m_gameClients[i].GetIsActive() == false)
		{
			m_gameClients[i].SetPlayerData(clientData, i + 1);

			// Report to the game we added a player
			ASSERT(GetGame() != NULL);
			GetGame()->OnAddNetworkPlayer(i + 1, &m_gameClients[i], clientData->color);
			return &(m_gameClients[i]);
		}
	}

	return 0;
}

// Remove a client
bool SockNetworkManager::RemoveClient(NetClient* client, bool migrate, bool bGameOver)
{
	// Client didn't resolve
	if (client == 0 || client->GetIsActive() == false) return false;

	DEBUG_PRINT("SockNetMan : RemoveClient (%s)\n", client->GetName());

	if(!bGameOver)
	{
		NetworkGame* pGame = GetGame();

		// Tell the game we removed this player
		if(pGame != NULL)
		{
			pGame->OnRemovePlayer(client->GetPlayerID());
		}
	}

	// See if we need to handle host migration
	bool wasHostThatLeft = (((SockClient*)client)->GetUID() == m_hostID);

	// Disconnect this client (we need the UID one line up before we erase it)
	client->Disconnect();

	if (migrate && wasHostThatLeft)
	{
		// See if we are the next host
		if (m_nextHostList[0] == GetUID())
		{
			m_hostID = GetUID();
			m_host = 0;

			// This is a lobby
			m_netState = NS_HostingLobby;

			// Tell the connection we migrated as the new host
			m_gameConnection.MigrateToLobby(GenerateGameUID(), true);
			m_hostListNextSend = NET_HOST_LIST_NEXT_SEND_TIME / 2.0f;

			bool isSending = false;

			// Mark all clients as not-yet-migrated
			for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
			{
				if (m_gameClients[i].GetRequestedUsAsHost())
				{
					NetMessageNewHostResponse response;
					response.hostID = GetUID();
					response.newGameID = GetGameID();
					m_gameClients[i].SendNetMessage(response, true);
					isSending = true;
				}
				if (m_gameClients[i].GetIsActive())
					m_gameClients[i].MarkNeedsToMigrate(true);
			}
			return isSending;
		}


		// Otherwise, starting from the top, ask the next one down the list if they are the host (if they are still connected)
		NetMessageNewHostRequest request;
		int i = 0;
		for (i = 0; i < USAGI_NET_MAX_CLIENTS - 1; i++)
		{
			sint64 nextHost = m_nextHostList[i];
			if (nextHost == GetUID())
				break; // Once you reach yourself, stop

			SendToClient(nextHost, request, true);
		}
		m_hostID = NET_CLIENT_INVALID_ID;
		m_host = 0;

		return true;
	}

	return false;
}

void SockNetworkManager::SetHost(sint64 hostID)
{
	m_hostID = hostID;
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		if (m_gameClients[i].GetUID() == m_hostID)
		{
			m_host = &m_gameClients[i];
			return;
		}
	}
}

// Fill the host name buffer
void SockNetworkManager::FillName(char* nameOut)
{
	// Get the host's name

	// Fill it
#if defined(PLATFORM_PC)
	char username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserName(username, &username_len);

	sprintf_s(nameOut, 32, "PC#%s", username);
#elif defined(PLATFORM_OSX)
	const char* username = getenv("USER");
	if (username == 0)
		sprintf(nameOut, "OSX#%i%i", (int)(m_UID >> 32), (int)(m_UID & 0xffffffff));
	else
		sprintf(nameOut, "OSX#%s", username);
#else
	sprintf(nameOut, "Unknown%i%i", (int)(m_UID >> 32), (int)(m_UID & 0xffffffff));
#endif
}

}