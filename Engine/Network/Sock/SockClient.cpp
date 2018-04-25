/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/BuildID.h"
#include "Engine/Network/NetPlatform.h"
#include "SockClient.h"
#include "SockClientLink.h"
#include "SockNetworkManager.h"
#include "../NetManager.h"
#include "../NetMessage.h"

namespace usg {

SockClient::SockClient()
{
	Disconnect();
}

SockClient::~SockClient()
{
	Disconnect();
}

void SockClient::Disconnect()
{
	DEBUG_PRINT("SockClient : Disconnect\n");

	m_needsToMigrate = false;
	m_requestedUsAsHost = false;

	m_ipAddress = 0;
	m_port = 0;
	m_UID = 0;

	NetClient::Disconnect();
}

void SockClient::SetPlayerData(NetSimplePlayerData* playerData, int playerID)
{
	if (playerData == 0)
	{
		m_port = 0;
		m_ipAddress = 0;
		m_UID = 0;
		SetColor(0);
		SetPlayerIndex(playerID);
		return;
	}

	m_port = playerData->port;
	m_ipAddress = playerData->ip;
	m_UID = playerData->uid;
	SetName(playerData->name);
	SetColor(playerData->color);
	SetPlayerIndex(playerID);
	SetActive(true);
}

void SockClient::CheckIncomingPacketForMigration(NetPacket* inPacket)
{
	// If this is during host migration, see if this came in with the new game ID
	if (m_needsToMigrate)
	{
		if (inPacket->header.identGame == SockNetworkManager::Inst()->GetGameID())
		{
			m_needsToMigrate = false;
		}
	}

}

bool SockClient::CheckForTimeout()
{
	bool needsToSendMessage = false;
	return needsToSendMessage;
}

void SockClient::SetOutgoingPacketHeader(NetPacket* packet)
{
	// Header data
	packet->IP = m_ipAddress;
	packet->port = m_port;
	packet->header.uIdentifier = (USAGI_BUILD_ID & 0xffff);
	packet->header.clientUID = NetManager::Inst()->GetUID();
	packet->header.identGame = SockNetworkManager::Inst()->GetGameID();

	// If we are migrating, send to the appropriate game id
	if (m_needsToMigrate)
		packet->header.identGame = SockGameConnection::Inst()->GetOldMigrationID();
}


void SockClient::SendPacketToMulti()
{
	// Send packets
	for (int i = 0; i <= m_packetQueueIndex; i++)
	{
		if (i == NET_PACKET_QUEUE_SIZE) break;

		NetPacket* packet = &m_packetOutQueue[i];

		packet->header.clientUID = NetManager::Inst()->GetUID();
		packet->header.lastReliablePacketRecvd = -1;
		packet->header.identGame = SockNetworkManager::Inst()->GetGameID();
		packet->header.uIdentifier = USAGI_BUILD_ID & 0xfff;
		packet->header.SetEndianess(net_get_endian_value());

		if (packet->header.numMessages == 0)
			return;

		SockNetworkManager::Inst()->SendOnMultiLink(packet);
		AddToBandwidth(packet->header.dataLength + sizeof(NetPacketHeader));

		memset(packet, 0, sizeof(NetPacket));
	}
	m_packetQueueIndex = 0;
	ResetTimeToSend();
}


// handle network-level messages here
void SockClient::ReadMessage(NetMessage* message)
{
	// Read and handle incoming messages
	switch (message->GetType())
	{
	case EMT_NEW_HOST_REQUEST:
	{
		NetMessageNewHostResponse response;
		// response.hostID = SockNetworkManager::Inst()->GetHost();

		// If we are the new host
		if (SockNetworkManager::Inst()->IsHost())
		{
			response.newGameID = SockNetworkManager::Inst()->GetGameID();
			SendNetMessage(response, true);
		}
		else
		{
			m_requestedUsAsHost = true;

			// Resets the internal timers
			SetActive(true);
		}

		break;
	}
	case EMT_NEW_HOST_RESPONSE:
	{
		NetMessageNewHostResponse* response = (NetMessageNewHostResponse*)message;
		if (response->hostID != NET_CLIENT_INVALID_ID)
		{
			// New host found. update the lobby
			SockGameConnection::Inst()->MigrateToLobby(response->newGameID, false);
			SockNetworkManager::Inst()->SetHost(response->hostID);
		}
		break;
	}
	default:
		NetClient::ReadMessage(message);
		return;
	}

}

}