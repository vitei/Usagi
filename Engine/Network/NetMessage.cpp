/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/

#include "Engine/Common/Common.h"
#include <Engine/Debug/Rendering/DebugRender.h>
#include <Engine/Network/NetManager.h>
#include "Engine/Network/NetworkGame.h"
#include "NetMessage.h"
#include "UsagiNetwork.h"

namespace usg {
	
void NetMessage::SmartSwap()
{
	// Swap the type, as this is pretty important
	net_endian_swap(m_type);

	// Swap the rest of the packet
	switch (m_type)
	{
		SMART_SWAP(PacketAck, EMT_PACKET_ACK);
		SMART_SWAP(PingClient, EMT_PING);
		SMART_SWAP(PongClient, EMT_PONG);
		SMART_SWAP(HostReport, EMT_HOST_REPORT);
		SMART_SWAP(ClientReport, EMT_CLIENT_REPORT);
		SMART_SWAP(JoinGame, EMT_JOIN_GAME);
		SMART_SWAP(LeaveGame, EMT_LEAVE_GAME);
		SMART_SWAP(NewClient, EMT_NEW_CLIENT);
		SMART_SWAP(GameAvailable, EMT_GAME_AVAILABLE);
		SMART_SWAP(HostList, EMT_NEW_HOST_LIST);
		SMART_SWAP(ClientLostRequest, EMT_CLIENT_LOST_REQUEST);
		SMART_SWAP(ClientLostResponse, EMT_CLIENT_LOST_RESPONSE);
		SMART_SWAP(NewHostRequest, EMT_NEW_HOST_REQUEST);
		SMART_SWAP(NewHostResponse, EMT_NEW_HOST_RESPONSE);
		SMART_SWAP(TimeRequest, EMT_TIME_REQUEST);
		SMART_SWAP(TimeReturn, EMT_TIME_RETURN);

		///////////////////////////////////////
		// add SMART_SWAP for your type here //
		///////////////////////////////////////


	default:
		ASSERT(false && "Message type not defined or packet was corrupt");
		break;
	}
}

void NetMessage::CreateMessageFromBuffer()
{
	NetMessage* n = this;
	switch (GetType())
	{
		CREATE_MESSAGE_FROM_BUFFER(PacketAck, EMT_PACKET_ACK);
		CREATE_MESSAGE_FROM_BUFFER(PingClient, EMT_PING);
		CREATE_MESSAGE_FROM_BUFFER(PongClient, EMT_PONG);
		CREATE_MESSAGE_FROM_BUFFER(HostReport, EMT_HOST_REPORT);
		CREATE_MESSAGE_FROM_BUFFER(ClientReport, EMT_CLIENT_REPORT);
		CREATE_MESSAGE_FROM_BUFFER(JoinGame, EMT_JOIN_GAME);
		CREATE_MESSAGE_FROM_BUFFER(LeaveGame, EMT_LEAVE_GAME);
		CREATE_MESSAGE_FROM_BUFFER(NewClient, EMT_NEW_CLIENT);
		CREATE_MESSAGE_FROM_BUFFER(GameAvailable, EMT_GAME_AVAILABLE);
		CREATE_MESSAGE_FROM_BUFFER(HostList, EMT_NEW_HOST_LIST);
		CREATE_MESSAGE_FROM_BUFFER(ClientLostRequest, EMT_CLIENT_LOST_REQUEST);
		CREATE_MESSAGE_FROM_BUFFER(ClientLostResponse, EMT_CLIENT_LOST_RESPONSE);
		CREATE_MESSAGE_FROM_BUFFER(NewHostRequest, EMT_NEW_HOST_REQUEST);
		CREATE_MESSAGE_FROM_BUFFER(NewHostResponse, EMT_NEW_HOST_RESPONSE);
		CREATE_MESSAGE_FROM_BUFFER(TimeRequest, EMT_TIME_REQUEST);
		CREATE_MESSAGE_FROM_BUFFER(TimeReturn, EMT_TIME_RETURN);

		///////////////////////////////////////////////////////
		// add CREATE_MESSAGE_FROM_BUFFER for your type here //
		///////////////////////////////////////////////////////

	default:
		DEBUG_PRINT("CreateMessageFromBuffer() for message type %u was not defined!\n", (uint32)GetType());
		break;
	}
}

NetMessage* CreateMessageFromBuffer(NetPacket* packet, sint16 offset, bool endianSwap)
{
	const sint16* messageHeader = (const sint16*)(packet->data + offset);
	sint16 isPB = messageHeader[0];
	sint16 type = messageHeader[1];
	sint16 size = messageHeader[2];
	if (endianSwap)
	{
		net_endian_swap(type);
		net_endian_swap(size);
	}

	// The size does not include the vfptr size, so include a buffer for it
	NetworkGame* game = NetManager::Inst()->GetGame();
	NetMessage* n = NULL;

	if(game != NULL)
	{
		n = game->InitialiseMessage( NetManager::Inst()->AllocateMessage(size + 24) );
	}
	else
	{
		void* mem = NetManager::Inst()->AllocateMessage(size + 24);
		n = new (mem) NetMessage(0);
	}

	ASSERT(n != NULL);
	n->Unpack(packet, offset, size);
	n->CreateMessageFromBuffer();

	ASSERT(size == n->GetLength());
	return n;
}

uint32 NetMessage::Pack(NetPacket* packet)
{
	return packet->Pack(&m_isPB, messageLength, GetType());
}

void NetMessage::Unpack(NetPacket* packet, sint16 offset, sint16 length)
{
	packet->Unpack(&m_isPB, offset, length, GetType());
}

// Handle this in blocks of 1, 2, 4, and 8-bytes
void net_swap_1(uint8*)
{
	// No processing needed
}

// Follow swap for strict-antialiasing rules
void net_swap_2(uint8* var)
{
	uint8 temp = var[0];
	var[0] = var[1];
	var[1] = temp;
}

// Follow swap for strict-antialiasing rules
void net_swap_4(uint8* val)
{
	uint8 temp[2] = { val[0], val[1] };
	val[0] = val[3];
	val[1] = val[2];
	val[2] = temp[1];
	val[3] = temp[0];
}

// Follow swap for strict-antialiasing rules
void net_swap_8(uint8* val)
{
	uint8 temp[4] = { val[0], val[1], val[2], val[3] };
	val[0] = val[7];
	val[1] = val[6];
	val[2] = val[5];
	val[3] = val[4];
	val[4] = temp[3];
	val[5] = temp[2];
	val[6] = temp[1];
	val[7] = temp[0];
}

#define DEFINE_ENDIAN_SWAP( type, size ) \
	void net_endian_swap( type &val) { net_swap_##size ((uint8*) &val ); } \

DEFINE_ENDIAN_SWAP(bool, 1)
DEFINE_ENDIAN_SWAP(sint8, 1)
DEFINE_ENDIAN_SWAP(sint16, 2)
DEFINE_ENDIAN_SWAP(sint32, 4)
DEFINE_ENDIAN_SWAP(sint64, 8)
DEFINE_ENDIAN_SWAP(uint8, 1)
DEFINE_ENDIAN_SWAP(uint16, 2)
DEFINE_ENDIAN_SWAP(uint32, 4)
DEFINE_ENDIAN_SWAP(uint64, 8)
DEFINE_ENDIAN_SWAP(float, 4)
DEFINE_ENDIAN_SWAP(double, 8)

void net_endian_swap(Vector3f& val)
{
	net_swap_4((uint8*)&val.x);
	net_swap_4((uint8*)&val.y);
	net_swap_4((uint8*)&val.z);
}

void net_endian_swap(Vector4f& val)
{
	net_swap_4((uint8*)&val.x);
	net_swap_4((uint8*)&val.y);
	net_swap_4((uint8*)&val.z);
	net_swap_4((uint8*)&val.w);
}

void net_endian_swap(Quaternionf& val)
{
	net_swap_4((uint8*)&val.x);
	net_swap_4((uint8*)&val.y);
	net_swap_4((uint8*)&val.z);
	net_swap_4((uint8*)&val.w);
}

}
