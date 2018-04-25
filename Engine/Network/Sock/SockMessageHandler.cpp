/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Network/NetPlatform.h"
#include "SockMessageHandler.h"
#include "SockNetworkManager.h"
#include <Engine/Network/NetPacket.h>
#include <Engine/Network/NetMessage.h>
#include <Engine/Core/BuildID.h>

namespace usg
{

bool SockMessageHandler::IsPacketValid(NetPacket* inPacket)
{
	// Check the header
	NetPacketHeader* header = &inPacket->header;

	// Sanity check that it's from an Usagi client
	if (header->uIdentifier != (USAGI_BUILD_ID&0xffff))
	{
		return false;
	}

	// Don't read packets from oneself
	if (header->clientUID == NetManager::Inst()->GetUID())
	{
		return false;
	}
#ifdef NET_DEBUG_ALLOW_PACKET_INSPECTION

	// These messages were not rec'd
	NetMessage* message = 0;
	int packetOffset = 0;
	int numMessagesToRead = inPacket->header.numMessages;
	bool endianSwap = (inPacket->header.parentEndianess != net_get_endian_value());

	while (packetOffset <= inPacket->header.dataLength &&
		numMessagesToRead > 0)
	{
		message = CreateMessageFromBuffer(inPacket->data + packetOffset, endianSwap);
		SockNetworkManager::Inst()->DebugMessageTypeReceived(message, 2);
		packetOffset += message->GetLength();

		NetManager::Inst()->FreeMessage(message);
		numMessagesToRead--;
	}
#endif
	// Check game id
	sint64 currentGame = SockNetworkManager::Inst()->GetGameID();
	if (header->identGame != NET_GAME_INVALID_GAME_ID)
	{
		// If not in a game, this could be a session packet for joining a new game
		if (currentGame == NET_GAME_INVALID_GAME_ID)
			return true;

		sint64 oldMigration = SockGameConnection::Inst()->GetOldMigrationID();
		if (header->identGame == oldMigration ||
			header->identGame == currentGame)
		{
			return true;
		}
		return false;
	}

	// It was a valid multicast packet
	return true;
}

void SockMessageHandler::ReadSessionMessage(NetMessage* message, NetPacket* inPacket)
{
	SockClient* client = SockNetworkManager::Inst()->GetClientFromPacket(inPacket);
	// Only handle these ones
	switch (message->GetType())
	{
	case EMT_LEAVE_GAME:
	{
		// Remove this client
		SockNetworkManager::Inst()->RemoveClient(client, false);
		break;
	}
	case EMT_NEW_CLIENT:
	{
		NetMessageNewClient* nmnc = (NetMessageNewClient*)message;
		if (nmnc->gameUID == SockNetworkManager::Inst()->GetGameID())
		{
			SockNetworkManager::Inst()->AddClient(&nmnc->playerData);
		}
		break;
	}
	case EMT_JOIN_GAME:
		SockNetworkManager::Inst()->m_gameConnection.OnClientWantsToJoin((NetMessageJoinGame*)message, inPacket);
		break;
	case EMT_GAME_AVAILABLE:
		SockNetworkManager::Inst()->m_gameConnection.OnGameAvailable((NetMessageGameAvailable*)message, inPacket);
		break;
	case EMT_NEW_HOST_LIST:
	{
		SockNetworkManager::Inst()->SyncHostList((NetMessageHostList*)message);
		break;
	}

	default:
		NetMessageHandler::ReadSessionMessage(message, inPacket);
		return;
	}
}


void SockMessageHandler::SendPacketToClients(NetPacket* packet)
{
	// If this was not a game packet, don't read messages internally
	if (packet->header.identGame == NET_GAME_INVALID_GAME_ID)
		return;

	for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
	{
		SockClient* client = (SockClient*)SockNetworkManager::Inst()->GetClient(i);
		if (client->GetUID() == packet->header.clientUID)
		{
			client->OnPacketReceived(packet);
			return;
		}
	}
}

}