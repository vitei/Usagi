/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_MESSAGE_H
#define NET_MESSAGE_H


#include "Engine/Maths/Vector3f.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Maths/Quaternionf.h"
#include "Engine/Network/NetMessage.pb.h"

#pragma pack(push, 4)

#define USAGI_NET_MAX_CLIENTS (8)
#define USAGI_NET_MAX_ENEMIES (10)
#define USAGI_NET_CLIENT_MAX_NAME_LENGTH (32)

#define DECLARE_ENDIAN_SWAP(type) \
	void net_endian_swap ( type & val );

#define SWAP_MESSAGE virtual void Swap() { ::usg::NetMessage::Swap();
#define SWAP_END }

#define SWAP( var) ::usg::net_endian_swap( var )

#define CONSTRUCT_NET_MESSAGE(MessageName, MessageType) \
struct NetMessage##MessageName : public ::usg::NetMessage { \
	NetMessage##MessageName () : ::usg::NetMessage( (MessageType) ) { messageLength = GetRawLength(); }

#define MESSAGE_END public: int lastXXXXX; \
	uint16 GetRawLength()  {	\
	uint8* toLast = (uint8*)(&lastXXXXX);	\
	uint8* toPB = (uint8*)(&m_isPB);		\
	return (uint16)(toLast - toPB); } \
	void SetRawLength() { messageLength = GetRawLength(); }};

namespace usg
{

enum ENetMessageTypes
{
	///////////////////////////////////////////
	// DON'T ADD TO THE MIDDLE OF THIS ENUM! //
	///////////////////////////////////////////

	EMT_NONE = 0,
	EMT_XOR = 0,

	EMT_PACKET_ACK,
	EMT_PING,
	EMT_PONG,

	EMT_HOST_REPORT,
	EMT_CLIENT_REPORT,

	EMT_GAME_AVAILABLE,
	EMT_NEW_HOST_LIST,
	EMT_CLIENT_LOST_REQUEST,
	EMT_CLIENT_LOST_RESPONSE,
	EMT_NEW_HOST_REQUEST,
	EMT_NEW_HOST_RESPONSE,

	EMT_JOIN_GAME,

	EMT_LEAVE_GAME,
	EMT_NEW_CLIENT,

	EMT_TIME_REQUEST,
	EMT_TIME_RETURN,

	//////////////////////
	// ADD HERE INSTEAD //
	//////////////////////

	EMT_NUM_NET_MESSAGE_TYPES
};

DECLARE_ENDIAN_SWAP(bool)
DECLARE_ENDIAN_SWAP(sint8)
DECLARE_ENDIAN_SWAP(sint16)
DECLARE_ENDIAN_SWAP(sint32)
DECLARE_ENDIAN_SWAP(sint64)
DECLARE_ENDIAN_SWAP(uint8)
DECLARE_ENDIAN_SWAP(uint16)
DECLARE_ENDIAN_SWAP(uint32)
DECLARE_ENDIAN_SWAP(uint64)
DECLARE_ENDIAN_SWAP(float)
DECLARE_ENDIAN_SWAP(double)
DECLARE_ENDIAN_SWAP(Vector3f)
DECLARE_ENDIAN_SWAP(Vector4f)
DECLARE_ENDIAN_SWAP(Quaternionf)

#define SMART_SWAP( messageName , enumType ) \
	case enumType : \
	((NetMessage##messageName *) this ) -> Swap(); \
	break

#define CREATE_MESSAGE_FROM_BUFFER( messageName, enumType ) \
	case enumType : ((NetMessage##messageName *)n)->SetRawLength(); break

struct NetMessage
{
public:
	NetMessage(sint16 t){
		m_isPB = 0;
		m_type = t;
	}

	ENetMessageTypes GetType() { return (ENetMessageTypes)m_type; }
	sint16 GetLength() { return messageLength; }
	sint32 GetReliableIndex() { return messageReliableIndex; }
	void SetReliableIndex(sint32 index) { messageReliableIndex = index; }

	uint32 Pack(struct NetPacket* packet);
	void Unpack(struct NetPacket* packet, sint16 offset, sint16 size);

	virtual void CreateMessageFromBuffer();
protected:
	virtual void Swap()
	{
		SWAP(messageLength);
		SWAP(messageReliableIndex);
	}
	sint16 m_isPB; // This MUST be the 1st data member
	sint16 m_type; // This MUST be the 2nd data member
	sint16 messageLength; // This MUST be the 3rd data member
private:
	sint32 messageReliableIndex;

protected:
	virtual void SmartSwap();
	friend class NetMessageHandler; // Access to SmartSwap()
};
// Additional messages...

class NetworkGame;
NetMessage* CreateMessageFromBuffer(struct NetPacket* packet, sint16 offset, bool endianSwap);

// Acknowledge that data has been received
CONSTRUCT_NET_MESSAGE(PacketAck, EMT_PACKET_ACK)
MESSAGE_END

// Ping the client
CONSTRUCT_NET_MESSAGE(PingClient, EMT_PING)
	double timeSent;

SWAP_MESSAGE
	SWAP(timeSent);
SWAP_END

MESSAGE_END

// Pong the client
CONSTRUCT_NET_MESSAGE(PongClient, EMT_PONG)
	double timeSent;

SWAP_MESSAGE
	SWAP(timeSent);
SWAP_END
MESSAGE_END

// Tell clients we are the host
CONSTRUCT_NET_MESSAGE(HostReport, EMT_HOST_REPORT)
MESSAGE_END

// Tell clients we are joining
CONSTRUCT_NET_MESSAGE(ClientReport, EMT_CLIENT_REPORT)
	uint8 color;
	uint32 uType;
MESSAGE_END

// Client requesting to join a game
CONSTRUCT_NET_MESSAGE(JoinGame, EMT_JOIN_GAME)
	sint64 gameUID;
	char userName[USAGI_NET_CLIENT_MAX_NAME_LENGTH];
	uint8 color;

	SWAP_MESSAGE
		SWAP(gameUID);
		SWAP(color);
	SWAP_END
		MESSAGE_END

// Client requesting to leave the game
CONSTRUCT_NET_MESSAGE(LeaveGame, EMT_LEAVE_GAME)
	sint64 gameUID;

SWAP_MESSAGE
	SWAP(gameUID);
SWAP_END
MESSAGE_END

// New client has joined the game
CONSTRUCT_NET_MESSAGE(NewClient, EMT_NEW_CLIENT)
	sint64 gameUID;
	NetSimplePlayerData playerData;
	sint16 playerIndex;

	SWAP_MESSAGE
		SWAP(gameUID);
		SWAP(playerIndex);
		SWAP(playerData.uid);
		SWAP(playerData.ip);
		SWAP(playerData.port);
		SWAP(playerData.color);
		SWAP(playerData.active);
	SWAP_END
MESSAGE_END

// Game available
CONSTRUCT_NET_MESSAGE(GameAvailable, EMT_GAME_AVAILABLE)
	
	// ID
	sint64 gameUID;

	// game data
	sint32 gameIsAvailable;

	// Connected clients
	NetSimplePlayerData client[USAGI_NET_MAX_CLIENTS];	

	SWAP_MESSAGE
		SWAP(gameUID);
		SWAP(gameIsAvailable);
		for (int i = 0; i < USAGI_NET_MAX_CLIENTS; i++)
		{
			SWAP(client[i].uid);
			SWAP(client[i].ip);
			SWAP(client[i].port);
			SWAP(client[i].color);
			SWAP(client[i].active);
		}
	SWAP_END

MESSAGE_END

// Host Migration host list
CONSTRUCT_NET_MESSAGE(HostList, EMT_NEW_HOST_LIST)
	double serverTime;
	sint64 hostList[USAGI_NET_MAX_CLIENTS-1];

	SWAP_MESSAGE
		SWAP(serverTime);
		for (int i = 0; i < USAGI_NET_MAX_CLIENTS - 1; i++)
			SWAP(hostList[i]);
	SWAP_END
MESSAGE_END

// Ask host if a client dropped
CONSTRUCT_NET_MESSAGE(ClientLostRequest, EMT_CLIENT_LOST_REQUEST)
	double serverTime;
	sint64 clientID;

	SWAP_MESSAGE
		SWAP(serverTime);
		SWAP(clientID);
	SWAP_END
MESSAGE_END

// Respond to a request that a client dropped
CONSTRUCT_NET_MESSAGE(ClientLostResponse, EMT_CLIENT_LOST_RESPONSE)
	sint64 clientID;
	bool isActive;

	SWAP_MESSAGE
		SWAP(clientID);
		SWAP(isActive);
	SWAP_END

MESSAGE_END

// Ask a client if they are going to be the new host
CONSTRUCT_NET_MESSAGE(NewHostRequest, EMT_NEW_HOST_REQUEST)
MESSAGE_END

// Respond to a client that we are or are not the new host
CONSTRUCT_NET_MESSAGE(NewHostResponse, EMT_NEW_HOST_RESPONSE)
	sint64 hostID;
	sint64 newGameID;

	SWAP_MESSAGE
		SWAP(hostID);
		SWAP(newGameID);
	SWAP_END
MESSAGE_END

// Time synchronization
CONSTRUCT_NET_MESSAGE(TimeRequest, EMT_TIME_REQUEST)
	double currentTime;

	SWAP_MESSAGE
		SWAP(currentTime);
	SWAP_END
MESSAGE_END

CONSTRUCT_NET_MESSAGE(TimeReturn, EMT_TIME_RETURN)
	double clientTime;
	double serverTime;

	SWAP_MESSAGE
		SWAP(clientTime);
		SWAP(serverTime);
	SWAP_END
MESSAGE_END

///////////////////////////////////////////////////////////
// DON'T FORGET TO IMPLEMENT NetMessage::SmartSwap() AND //
// CreateMessageFromBuffer() IN THE NetMessage.cpp FILE! //
///////////////////////////////////////////////////////////

}

#pragma pack(pop)

#endif
