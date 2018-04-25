/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_PACKET_H
#define NET_PACKET_H

#define NETWORK_PACKET_MAX_LENGTH		(1164)	// MTU value

#define NETWORK_PACKET_RESEND_TIME		(.5f)

#define CLIENT_LINK_LOCAL_PORT			(2946) // Arbitrary
#define NET_TV_TIMEOUT					(500) // microseconds to wait before sending

#define MULTICAST_TTL					(3) // How many routers we want to look through
#define MULTICAST_IP_ADDRESS			("226.65.231.6") // Arbitrary, in MC range
#define MULTICAST_PORT					(6423) // Arbitrary

#include "Engine/Network/NetCommon.h"
#include "NetPlatform.h"
#include "NetMessage.h"

namespace usg
{

struct NetPacketHeader
{
private:
	enum
	{
		BIT_ENDIANESS = 1,
	};
public:
	// Identifying information
	sint64 identGame;
	sint64 clientUID;
	uint16 uIdentifier;

	uint8 uBitmask;

	// RUDP header
	int lastReliablePacketRecvd;

	// Number of messages inside
	sint16 dataLength;
	sint16 origLength;
	sint8 numMessages;
	uint8 compression;
	
	void SetEndianess(sint16 iValue)
	{
		ASSERT(iValue >= 0 && iValue <= 1);
		if (iValue == 0)
		{
			uBitmask &= ~(BIT_ENDIANESS);
		}
		else
		{
			uBitmask |= BIT_ENDIANESS;
		}
	}

	sint16 GetEndianess() const
	{
		return (uBitmask & BIT_ENDIANESS) != 0 ? 1 : 0;
	}

	void Swap()
	{
		SWAP(identGame);
		SWAP(clientUID);
		SWAP(uIdentifier);
		SWAP(lastReliablePacketRecvd);
		SWAP(uBitmask);
		SWAP(dataLength);
		SWAP(origLength);
		SWAP(numMessages);
		SWAP(compression);
	}
};

struct NetPacket
{
	// Header
	NetPacketHeader header;
	uint8 data[NETWORK_PACKET_MAX_LENGTH - sizeof(NetPacketHeader)];

	///////////////////////////////////////////////
	// ANYTHING BELOW data[] IS NOT TRANSMITTED. //
	///////////////////////////////////////////////
	
	// Sanity check that the data we received matches what's in the packet
	sint16 length;

	// For data compression
	uint32 m_lastPackedMessageType;
	uint8* m_lastPacketMessageLocation;

	// Identifying information
	unsigned long IP;
	short port;	

	uint32 Pack(void* data_in, size_t data_length, uint32 type);
	void Unpack(void* data_out, sint16 offset, sint16 data_length, uint32 type);

	NetPacket();
};

}

#endif
