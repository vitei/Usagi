/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include "NetCommon.h"
#include "NetClient.h"
#include "NetClientLink.h"
#include "NetMultiLink.h"

#include <Engine/Memory/MemHeap.h>

#include "NetTime.h"
#include "NetGameConnection.h"
#include "NetMessageHandler.h"

namespace usg
{
	class MessageDispatch;
	struct MessageDispatchShutdown;

#define USAGI_NET_MAX_CLIENTS			(8)
#define NET_MAN_NET_DEBUG_LINES			(20)

#define NET_CLIENT_INVALID_ID			(-1)
#define NET_CLIENT_MULTICAST			(NET_CLIENT_INVALID_ID)

//#define NET_DEBUG_ALLOW_PACKET_INSPECTION

// Network states
enum NetworkState
{
	NS_Disconnected,
	NS_HostingLobby,
	NS_JoiningLobby,
	NS_ClientLobby,
	NS_Disconnecting,
};

class NetworkGame;
class UsagiInetCore;

// Network class manager
class NetManager
{
public:
	NetManager(UsagiInetCore* pInetCore);
	virtual ~NetManager();

	// Return the internal instance of this class
	static NetManager* Inst(){ return Instance; }

	UsagiInetCore& GetInetCore();

	// Local host
	virtual SessionOpenForParticipationStatus GetOpenForParticipationStatus() const = 0;
	virtual bool SetOpenForParticipation(bool bOpen) = 0;

	// Basic operation
	virtual void SetUninitialized();
	virtual void Initialise();
	virtual void Update(float deltaTime);
	virtual void DisconnectAll(bool bGameOver = false) = 0;
	virtual void StartHosting() = 0;
	virtual void StartClient() = 0;
	static bool IsInitialized() { return (Inst() && Inst()->IsInitializedInt()); }
	bool IsInErrorState();
	
	static bool GenericIsHost();

	// Set a new context for the net manager
	void SetContext(NetworkGame* game, MessageDispatch* dispatch);

	// Sending data
	virtual void SendOnClientLink(NetPacket* packet) = 0;
	virtual void SendToMulti(NetMessage&){ }
	void SendToAll(NetMessage& message, bool bReliable, bool bIncludeSelf = false);
	void SendToClient(long long uid, NetMessage& message, bool bReliable);
	
	// Client related
	virtual NetClient* GetClient(int idx) = 0;
	virtual const NetClient* GetClient(int idx) const = 0;
	NetClient* AddClient(sint64 uid, uint32 ip, uint16 port, char* name);
	virtual bool RemoveClient(class NetClient* client, bool migrate, bool bGameOver = false) = 0;
	int GetNumConnectedClients();
	bool AreClientSlotsAvailable();
	uint16 GetTotalUnackedMessageCount() const;

	// Local information
	sint64 GetUID() const { return m_UID; }
	NetClient* GetHost() { return m_host; }
	bool HasHost() const { return m_host != NULL; }
	bool IsHost();

	// Utilities
	virtual void AttemptToJoinGame(struct NetAvailableGame* game) = 0;

	// Get available network games.
	virtual const AvailableGameInfoContainer& GetAvailableGames();

	// Connect to a network game asynchroneously. Returns true if connection process was started succesfully. Can fail if the given is incorrect.
	virtual bool ConnectToGame(uint32 uGameId);

	virtual bool IsConnectedToGame();

	static sint64 GenerateGameUID(); // Safe to use even in single player
	                                 // static, so works even when NetManager::Inst() is NULL
	NetMessage* AllocateMessage(int messageSizeBytes);
	void FreeMessage(NetMessage* message);
	void* AllocateBytes(memsize sizeBytes);
	void FreeBytes(void* data);

	NetworkGame* GetGame() { return m_game; }

	// Packet Inspection
#ifdef NET_DEBUG_ALLOW_PACKET_INSPECTION
	void DebugMessageTypeReceived(NetMessage* message, int recvType);
	void DebugMessageTypeSent(NetMessage* message);
	void DebugDisplayRecdMessages();
#else
	void DebugDisplayRecdMessages() { }
#endif

	// Utilities
	void UpdateUID();

protected:
	// Update
	virtual void UpdateHostingLobby(float deltaTime) = 0;
	virtual void UpdateClientLobby(float deltaTime) = 0;
	virtual void UpdateJoiningLobby(float deltaTime) = 0;
	bool IsInitializedInt() { return m_isInitialized; }

	// Internal getters
	virtual NetGameConnection* GetGameConnection() = 0;
	virtual NetClient* GetClientFromPacket(NetPacket* inPacket) = 0;

	// Handles syncing and keeping track of server time
	NetTime m_NetTime;

	// Identification
	NetClient* m_host;
	sint64 m_UID;
	NetworkState m_netState;
	bool m_isInitialized;
	bool m_bIsHost;

	// Message dispatch
	MessageDispatch* GetMessageDispatch() { return m_messageDispatch; }

private:
	// Message handling
	void HandleIncomingMessage(NetPacket* inPacket);
	void ReadPacket(NetPacket* inPacket);
	void ReadMessage(NetMessage* message, NetPacket* inPacket);

	MessageDispatch* m_messageDispatch;
	void OnDispatchShutdown(const MessageDispatchShutdown& evt);

	// Pointer to the internal instance
	static NetManager* Instance;

	// Heap for network purposes
	void *m_memPoolBuffer;
	MemHeap m_memPool;

	UsagiInetCore* m_pInetCore;
	NetworkGame* m_game;

	// Debug displaying packet contents on the screen
#ifdef NET_DEBUG_ALLOW_PACKET_INSPECTION
	ENetMessageTypes m_messageTypesSent[NET_MAN_NET_DEBUG_LINES];
	ENetMessageTypes m_messageTypesRecd[NET_MAN_NET_DEBUG_LINES];
	int m_messageRecdType[NET_MAN_NET_DEBUG_LINES];
	int m_messageTypeIndex;
	int m_messageTypeOutIndex;
#endif

	friend class NetGameConnection;
	friend class NetMessageHandler;
	friend class NetClient;
};

}

#endif
