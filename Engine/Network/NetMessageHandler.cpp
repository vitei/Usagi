/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "NetPlatform.h"
#include "NetManager.h"
#include "NetMessageHandler.h"
#include "NetPacket.h"
#include "NetMessage.h"
#include "NetTime.h"
#include "NetDataCompressor.h"

namespace usg
{

void NetMessageHandler::HandleIncomingPacket(NetPacket* inPacket)
{
	if (inPacket->length < sizeof(NetPacketHeader))
		return;
	
	// Potentially unpack
	if (inPacket->header.GetEndianess() != net_get_endian_value())
		inPacket->header.Swap();

	// Sizes didn't match (usually a sign of a much bigger problem)
	if (inPacket->length != inPacket->header.dataLength + sizeof(NetPacketHeader))
		return;

	// Check the header
	if (!IsPacketValid(inPacket))
	{
		return;
	}

	// Decompress packet
	NetDataCompressorUtil::DecompressPacket(inPacket);

	// Read it as a session packet first
	ReadSessionPacket(inPacket);

	// Handle clients
	SendPacketToClients(inPacket);	
}

void NetMessageHandler::ReadSessionPacket(NetPacket* inPacket)
{

	NetMessage* message = 0;
	sint16 packetOffset = 0;
	int numMessagesToRead = inPacket->header.numMessages;
	bool endianSwap = (inPacket->header.GetEndianess() != net_get_endian_value());
	while (packetOffset <= inPacket->header.dataLength &&
		numMessagesToRead > 0)
	{
		sint16 pbLengthHdr = *(sint16*)(inPacket->data + packetOffset);
		if(pbLengthHdr != 0)
		{
			if(endianSwap) { net_endian_swap(pbLengthHdr); }
			packetOffset += pbLengthHdr;
			numMessagesToRead--;
		}
		else
		{
			message = CreateMessageFromBuffer(inPacket, packetOffset, endianSwap);

			if (endianSwap)
			{
				// Unpack this message
				message->SmartSwap();
			}

			// Read the message in
			ReadSessionMessage(message, inPacket);
			packetOffset += message->GetLength();

			NetManager::Inst()->FreeMessage(message);
			numMessagesToRead--;
		}
	}
}


void NetMessageHandler::ReadSessionMessage(NetMessage* message, NetPacket* inPacket)
{
	NetClient* client = NetManager::Inst()->GetClientFromPacket(inPacket);
	// Only handle these ones
	switch (message->GetType())
	{
	case EMT_TIME_REQUEST:
	{
		if (client)
		{
			NetMessageTimeRequest* request = (NetMessageTimeRequest*)message;
			NetTime::Inst()->OnTimeRequest(request, client);
		}
		break;
	}
	case EMT_TIME_RETURN:
	{
		NetMessageTimeReturn* nmtr = (NetMessageTimeReturn*)message;
		NetTime::Inst()->OnTimeResponse(nmtr);
		break;
	}
	default:
		break;
	}

#ifdef NET_DEBUG_ALLOW_PACKET_INSPECTION
	NetManager::Inst()->DebugMessageTypeReceived(message, 1);
#endif

}

}
