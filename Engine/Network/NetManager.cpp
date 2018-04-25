/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Network/NetCommon.h"
#include "Engine/Debug/rendering/DebugRender.h"
#include "Engine/Core/Timer/Timer.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Core/Containers/Queue.h"
#include "NetPlatform.h"
#include "NetManager.h"
#include "NetMessage.h"
#include "UsagiNetwork.h"
#include "Engine/Framework/MessageDispatch.h"
#include "Engine/Network/NetworkGame.h"

namespace usg {

NetManager* NetManager::Instance = 0;

NetManager::NetManager(UsagiInetCore* pInetCore)
	: m_host(NULL)
	, m_bIsHost(false)
	, m_messageDispatch(NULL)
	, m_memPoolBuffer(NULL)
	, m_pInetCore(pInetCore)
	, m_game(NULL)
{
	Instance = this;
}

bool NetManager::IsHost()
{
	return m_bIsHost;
}

bool NetManager::IsInErrorState()
{
	return GetGameConnection()->IsInErrorState();
}

UsagiInetCore& NetManager::GetInetCore()
{
	ASSERT(m_pInetCore != nullptr);
	return *m_pInetCore;
}

void NetManager::SetUninitialized()
{
	m_isInitialized = false;
}

void NetManager::Initialise()
{
    static const size_t POOL_SIZE = 96 * 1024;
	m_memPoolBuffer = mem::Alloc(MEMTYPE_NETWORK, ALLOC_OBJECT, POOL_SIZE, 4U);
	m_memPool.Initialize(m_memPoolBuffer, POOL_SIZE);
	
#ifdef NET_DEBUG_ALLOW_PACKET_INSPECTION
	memset(m_messageTypesRecd, 0, sizeof(m_messageTypesRecd));
	memset(m_messageTypesSent, 0, sizeof(m_messageTypesSent));
	m_messageTypeIndex = 0;
	m_messageTypeOutIndex = 0;
#endif

	m_netState = NS_Disconnected;
	UpdateUID();
}

NetManager::~NetManager()
{
	if(m_memPoolBuffer)
	{
		mem::Free(MEMTYPE_NETWORK, m_memPoolBuffer);
	}
	Instance = 0;
}

// Generate a UID based on our game ID and the current time
sint64 NetManager::GenerateGameUID()
{
	if(NetManager::Inst())
	{
		double currentTime = net_get_time();
		return NetManager::Inst()->GetUID() ^ *(sint64*)(&currentTime);
	}
	else
	{
		return (sint64)net_get_time();
	}
}

#ifdef NET_DEBUG_ALLOW_PACKET_INSPECTION

void NetManager::DebugMessageTypeSent(NetMessage* message)
{
	switch (message->GetType())
	{
	case EMT_PACKET_ACK:
	case EMT_HELI_CONTROLLER_UPDATE:
	case EMT_GAME_AVAILABLE:
	case EMT_NEW_HOST_LIST:
		return;
	}

	m_messageTypesSent[m_messageTypeOutIndex] = message->GetType();
	m_messageTypeOutIndex = (m_messageTypeOutIndex + 1) % NET_MAN_NET_DEBUG_LINES;
	m_messageTypesSent[m_messageTypeOutIndex] = EMT_NONE;
}

void NetManager::DebugMessageTypeReceived(NetMessage* message, int recvType)
{
	switch (message->GetType())
	{
	case EMT_PACKET_ACK:
	case EMT_HELI_CONTROLLER_UPDATE:
	case EMT_NEW_HOST_LIST:
		return;
	}

	m_messageTypesRecd[m_messageTypeIndex] = message->GetType();
	m_messageRecdType[m_messageTypeIndex] = recvType;
	m_messageTypeIndex = (m_messageTypeIndex + 1) % NET_MAN_NET_DEBUG_LINES;
	m_messageTypesRecd[m_messageTypeIndex] = EMT_NONE;
}

void NetManager::DebugDisplayRecdMessages()
{
	int first = (m_messageTypeIndex + NET_MAN_NET_DEBUG_LINES - 1) % NET_MAN_NET_DEBUG_LINES;
	int last = (m_messageTypeIndex + NET_MAN_NET_DEBUG_LINES- 1);

	static char* messageTypeNames[] =
	{
		"",
		"Packet Ack",
		"Ping",
		"Pong",
		"Host Report",
		"Client Report",
		"Game Available",
		"New Host List",
		"Client Lost Request",
		"Client Lost Response",
		"New Host Request",
		"New Host Response",
		"Join Game",
		"Leave Game",
		"New Client",
		"Time Request",
		"Time Return"
	};
	Color white; white.Assign(.9f, 1, 1, 1);
	Color green; green.Assign(.5f, 1, .5f, 1);
	Color red; red.Assign(1, 0, 0, 1);
	Color recvColor[] = { white, green, red };


	float j = 1.0f;
	for (int i = 0; i < NET_MAN_NET_DEBUG_LINES; i++)
	{
		DebugRender::GetRenderer()->AddString(messageTypeNames[m_messageTypesRecd[i]], .7f, j, recvColor[m_messageRecdType[i]]);
		DebugRender::GetRenderer()->AddString(messageTypeNames[m_messageTypesSent[i]], .85f, j, recvColor[0]);
		j += 1.0f;
	}

}
#endif

void NetManager::Update(float deltaTime)
{
	// Early out if things didn't resolve
	if (m_isInitialized == false)
		return;

	if (GetGameConnection()->IsDisconnected())
	{
		m_netState = NS_Disconnected;
	}

	// Handle current game state
	switch (m_netState)
	{
	case NS_Disconnected:
		break;
	case NS_HostingLobby:
		UpdateHostingLobby(deltaTime);
		break;
	case NS_ClientLobby:
		UpdateClientLobby(deltaTime);
		break;
	case NS_JoiningLobby:
		UpdateJoiningLobby(deltaTime);
		break;
	case NS_Disconnecting:
		break;
	}

	sint64 lastUID = m_UID;
	UpdateUID();
	if(lastUID != m_UID) DEBUG_PRINT("Updated ID: 0x%016x => 0x%016x\n", lastUID, m_UID);

	// Update game connection management
	GetGameConnection()->Update(deltaTime);
}

// Allocate a message
NetMessage* NetManager::AllocateMessage(int messageNumBytes)
{	
	return (NetMessage*)AllocateBytes(messageNumBytes);
}

// Deallocate a message
void NetManager::FreeMessage(NetMessage* message)
{
	FreeBytes(message);
}

// Allocate some raw bytes
void* NetManager::AllocateBytes(memsize numBytes)
{
	void* pBytes = m_memPool.Allocate(numBytes, 4, 0, ALLOC_NETWORK);
#ifndef FINAL_BUILD
	if (pBytes == NULL)
	{
		DEBUG_PRINT("Failed to allocate %u bytes. Heap allocations currently %u bytes\n", numBytes, m_memPool.GetAllocated(ALLOC_NETWORK));
	}
#endif
	return pBytes;
}

// Deallocate bytes
void NetManager::FreeBytes(void* bytes)
{
	m_memPool.Deallocate(bytes);
}

bool NetManager::GenericIsHost()
{
	NetManager* pNetManager = NetManager::Inst();
	if (!pNetManager)
		return true;

	return (!pNetManager->IsInitialized() || pNetManager->IsHost());
}

// Set the handlers for various types of message (hopefully unify all this into MessageDispatch eventually)
void NetManager::SetContext(NetworkGame* game, MessageDispatch* dispatch)
{
	//The message dispatch and the game should have been cleared by
	//OnDispatchShutdown by the time we get here...
	ASSERT(m_messageDispatch == NULL && m_game == NULL);

	m_messageDispatch = dispatch;
	m_game = game;

	if(m_messageDispatch != NULL)
	{
		m_messageDispatch->RegisterCallback(this, &NetManager::OnDispatchShutdown);
	}
}

// Send a message to all connected clients
void NetManager::SendToAll(NetMessage& message, bool bReliable, bool bIncludeSelf)
{
	NetClient* pClient = NULL;

	for(int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		pClient = GetClient(i);
		ASSERT(pClient != NULL);

		if(pClient->GetIsActive())
		{
			pClient->SendNetMessage(message, bReliable);
		}
	}

	if(bIncludeSelf)
	{
		if(m_game)
		{
			GetGame()->OnMessageReceived(&message);
		}
		else
		{
			DEBUG_PRINT("Dropped local message of type %d because there is no active game\n", message.GetType());
		}
	}
}

// Send a message to a specific client
void NetManager::SendToClient(long long uid, NetMessage& message, bool bReliable)
{
	NetClient* pClient = NULL;

	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		pClient = GetClient(i);
		ASSERT(pClient != NULL);

		if(pClient->GetIsActive() && (pClient->GetUID() == uid))
		{
			pClient->SendNetMessage(message, bReliable);
			return;
		}
	}
}

// See if any client slots are available
bool NetManager::AreClientSlotsAvailable()
{
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		if (GetClient(i)->GetIsActive() == false)
			return true;
	}
	return false;
}

// Count up all the unacked required messages from clients
uint16 NetManager::GetTotalUnackedMessageCount() const
{
	sint16 count = 0;
	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		if (GetClient(i)->GetIsActive())
		{
			count += GetClient(i)->GetNumUnackedMessages();
		}
	}
	return count;
}

bool NetManager::IsConnectedToGame()
{
	return GetGameConnection()->IsConnectedToGame();
}

bool NetManager::ConnectToGame(uint32 uGameId)
{
	GetGameConnection()->ConnectToGame(uGameId);
	return true;
}

const AvailableGameInfoContainer& NetManager::GetAvailableGames()
{
	return GetGameConnection()->GetAvailableGames();
}

// Get the number of connected clients
sint32 NetManager::GetNumConnectedClients()
{
	sint32 iNum = 0;
	for (sint32 i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		if (GetClient(i) != nullptr && GetClient(i)->GetIsActive())
		{
			iNum++;
		}
	}
	return iNum;
}

// Save a global unique 64-bit number (MAC address)
void NetManager::UpdateUID()
{
	m_UID = net_get_uid();
}

void NetManager::OnDispatchShutdown(const MessageDispatchShutdown& evt)
{
	m_messageDispatch = NULL;

	// NULLing the game here as well as right now the game and the dispatch
	// generally have the same lifetime (i.e. a mode)
	// This is a bit nasty but I'm hoping the whole concept of a "game" will
	// go away (at least NetManager shouldn't need a pointer to it) as we
	// move away from NetMessages and toward using PB messages for everything.
	m_game = NULL;
}

}
