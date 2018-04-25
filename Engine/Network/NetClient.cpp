/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/BuildID.h"
#include "Engine/Framework/MessageDispatch.h"

#include "NetPlatform.h"
#include "NetClient.h"
#include "NetManager.h"
#include "NetMessage.h"
#include "NetworkGame.h"
#include "NetGameConnection.h"
#include "NetCommon.h"

namespace usg {

ReliableMessage::ReliableMessage()
	: isPB(false)
	, netMessage(NULL)
{
}

ReliableMessage::ReliableMessage(NetMessage* msg)
	: isPB(false)
	, netMessage(msg)
{
}

ReliableMessage::ReliableMessage(const PBMessage& msg)
	: isPB(true)
	, pbMessage(msg)
{
}

int ReliableMessage::GetReliableIndex() const
{
	if(isPB) { return pbMessage.reliable_index; }
	else { return netMessage->GetReliableIndex(); }
}

void ReliableMessage::Free()
{
	if(isPB) { NetManager::Inst()->FreeBytes(pbMessage.msg); }
	else { NetManager::Inst()->FreeMessage(netMessage); }
}

sint16 ReliableMessage::GetLength() const
{
	if(isPB) { return (sint16)pbMessage.length_bytes; }
	else { return netMessage->GetLength(); }
}

uint32 ReliableMessage::Pack(struct NetPacket* packet)
{
	if(isPB) { return packet->Pack(pbMessage.msg, pbMessage.length_bytes, pbMessage.type); }
	else { return netMessage->Pack(packet); }
}

NetClient::NetClient()
{
	m_bReceivedPong = true;
	Disconnect();
}

NetClient::~NetClient()
{
	Disconnect();
}

void NetClient::Disconnect()
{
	SetActive(false);

	// bandwidth
	m_totalBytesOut = 0;
	m_bandwidthTimer = 0;
	m_bandwidthIndex = 0;
	for (int b = 0; b < NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH + 1; b++)
	{
		m_bandwidthCounter[b] = 0;
		m_origBandwithCounter[b] = 0;
	}


	// Ping 
	m_bReceivedPong = true;
	m_pingQueueIndex = 0;
	m_pingAverage = 0;
	m_pingTimeBest = (unsigned int)(-1);
	m_nextPingTime = CLIENT_PING_TIME_SECONDS;
	for (int p = 0; p < NET_CLIENT_PING_QUEUE_LENGTH; p++)
		m_pingTimeQueue[p] = 0;

	// Clear outgoing packets
	memset(&m_packetOutQueue, 0, sizeof(NetPacket)* NET_PACKET_QUEUE_SIZE);
	m_packetQueueIndex = 0;
	ResetTimeToSend();
	
	// RUDP layer
	m_nextReliablePacketIndex = 1;
	m_lastReliableRecvdIndex = 0;
	m_lastAckedPacketIndex = -1;

	while (m_reliableMessagesOut.Count() > 0)
	{
		ReliableMessage message = m_reliableMessagesOut.pop();
		message.Free();
	}

#ifndef FINAL_BUILD
	ResetLengthSentReceived();
	memset(&m_uSentReliableAvg, 0, sizeof(m_uSentReliableAvg));
	memset(&m_uSentUnreliableAvg, 0, sizeof(m_uSentUnreliableAvg));
	memset(&m_uReceivedAvg, 0, sizeof(m_uReceivedAvg));
	m_uReceivedPeak = 0;
	m_uAvgWindowPos = 0;
#endif
}

void NetClient::SetName(char* name)
{
	for (int i = 0; i < 32; i++)
		m_name[i] = name[i];
}

void NetClient::CalculateBandwidth(float deltaTime)
{
	m_bandwidthTimer -= deltaTime;
	if (m_bandwidthTimer < 0)
	{
		m_bandwidthTimer += 1.0f;

		m_bandwidthCounter[m_bandwidthIndex] = m_bandwidthCounter[NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH];
		m_bandwidthCounter[NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH] = 0;
		m_origBandwithCounter[m_bandwidthIndex] = m_origBandwithCounter[NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH];
		m_origBandwithCounter[NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH] = 0;

		m_bandwidthIndex = (m_bandwidthIndex + 1) % NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH;
	}
}

void NetClient::Update(float deltaTime)
{	
	if (m_playerIndex == 0)
		return;

	// Calculate outgoing bandwidth
	CalculateBandwidth(deltaTime);	

	m_timeToSend -= deltaTime;
	bool sendPacket = m_timeToSend < 0;

	// If this goes to multicast, send it
	if (m_playerIndex == NET_CLIENT_MULTICAST)
	{
		// Send out
		if (sendPacket)
			SendPacketToMulti();
		return;
	}

	// Update single client-link
	if (CheckForTimeout())
		return;

	UpdatePing(deltaTime);

	if (sendPacket)
	{
		SendPacketToClient();
	}

#ifndef FINAL_BUILD
	m_uAvgWindowPos = (m_uAvgWindowPos + 1) % WINDOW_LENGTH;
#endif
}

float NetClient::GetAverageBandwidth()
{
	sint64 bandwidth = 0;
	for (int i = 0; i < NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH; i++)
	{
		bandwidth += m_bandwidthCounter[i];
	}
	return bandwidth / (float)NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH;
}

float NetClient::GetBandwidthCompressionRate()
{
	sint64 origBandwidth = 0;
	for (int i = 0; i < NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH; i++)
	{
		origBandwidth += m_origBandwithCounter[i];
	}
	float orig = origBandwidth / (float)NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH;
	return 1.0f - (GetAverageBandwidth() / orig);
}

#ifndef FINAL_BUILD
void NetClient::ResetLengthSentReceived()
{
	m_uSentReliable = 0;
	m_uSentUnreliable = 0;
	m_uReceived = 0;
}

float NetClient::GetAvg(uint32* pArray, uint32 uLen)
{
	uint32 uTotal = 0;

	for(uint32 i = 0; i < uLen; ++i)
	{
		uTotal += pArray[i];
	}

	return ((float)uTotal / (float)uLen);
}

float NetClient::GetAvgLengthSent(bool bReliable)
{
	if(bReliable)
	{
		return GetAvg(m_uSentReliableAvg, WINDOW_LENGTH);
	}

	return GetAvg(m_uSentUnreliableAvg, WINDOW_LENGTH);
}
#endif

void NetClient::SetActive(bool bActive)
{
	m_isActive = bActive;
	if (!bActive)
	{
		m_playerIndex = 0;
	}
	m_lastCommunicationTime = net_get_time() + 5.0;
	m_bReceivedPong = true;
	if (bActive)
	{
		m_fActivationTime = (float32)net_get_time();
		m_bReceivedClientReport = false;
	}
}

bool NetClient::CheckForTimeout()
{
	bool needsToSendMessage = false;
	// Subtracting 10 just to give a bit of a buffer...
	static const int unackedThreshold = RELIABLE_MESSAGE_QUEUE_LENGTH - 10;
	const uint32 uNumUnacked = GetNumUnackedMessages();
	if (uNumUnacked > unackedThreshold || NetGameConnection::Inst()->IsInErrorState())
	{
		// Remove this client (they have timed out)
		if (NetManager::Inst()->RemoveClient(this, true))
			needsToSendMessage = true;
	}
	return needsToSendMessage;
}

double NetClient::GetTimeSinceLastCommunication()
{
	return net_get_time() - m_lastCommunicationTime;
}

NetPacket* NetClient::VerifyPacketLength(NetPacket* packet, int length)
{
	while (packet->header.dataLength + length > sizeof(packet->data))
	{
	//	DEBUG_PRINT("used %i packets this update", m_packetQueueIndex + 1);
		m_packetQueueIndex++;
		if (m_packetQueueIndex >= NET_PACKET_QUEUE_SIZE)
			return 0;

		packet = &m_packetOutQueue[m_packetQueueIndex];
	}

	return packet;
}

void NetClient::PackReliableMessages()
{
	// All available packets are full
	if (m_packetQueueIndex >= NET_PACKET_QUEUE_SIZE)
	{
		ASSERT(false);
		return;
	}

	// Get the first packet
	NetPacket* packet = &m_packetOutQueue[m_packetQueueIndex];

	// Fit as many remaining reliable messages out onto this as will fit
	for (NetMessageQueue::Iterator it = m_reliableMessagesOut.Begin(); !it.IsEnd(); ++it)
	{
		ReliableMessage message = (*it);
		packet = VerifyPacketLength(packet, message.GetLength());
		if (packet == 0)
			return;

		packet->header.dataLength += message.Pack(packet);
		packet->header.numMessages++;
	}
}

void NetClient::SendPacketToClient()
{
	PackReliableMessages();

	for (int i = 0; i <= m_packetQueueIndex; i++)
	{
		if (i == NET_PACKET_QUEUE_SIZE) break;

		NetPacket* packet = &m_packetOutQueue[i];
		
		SetOutgoingPacketHeader(packet);

		packet->header.lastReliablePacketRecvd = m_lastReliableRecvdIndex;
		packet->header.SetEndianess(net_get_endian_value());

		if (packet->header.numMessages != 0)
		{
			// Send the packet to the client
			NetManager::Inst()->SendOnClientLink(packet);
			AddToBandwidth(packet->header.dataLength + sizeof(NetPacketHeader));
			m_bandwidthCounter[NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH] += packet->header.dataLength + sizeof(NetPacketHeader);
			m_origBandwithCounter[NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH] += packet->header.origLength + sizeof(NetPacketHeader);
		}

		// Sent, clear the packet
		memset(packet->data, 0, sizeof(packet->data));
		packet->header.numMessages = 0;
		packet->header.dataLength = 0;
	}
	m_packetQueueIndex = 0;
	ResetTimeToSend();
}
void NetClient::UpdatePing(float deltaTime)
{
	// Check the time
	m_nextPingTime -= deltaTime;
	if (m_nextPingTime < 0)
	{
		if (m_bReceivedPong)
		{
			m_bReceivedPong = false;
			m_nextPingTime = CLIENT_PING_TIME_SECONDS; // Long enough to wait but short enough to do it again
			NetMessagePingClient ping;
			ping.timeSent = net_get_time();
			SendNetMessage(ping, true);
		}
	}
}

NetMessage* NetClient::AllocateMessageCopy(NetMessage* message)
{
	// Copy message after the buffer (include length of vftable)
	NetMessage* messageCopy = (NetMessage*)NetManager::Inst()->AllocateMessage(message->GetLength() + 8);
	ASSERT(messageCopy != 0);
	SUPPRESS_WARNING("-Wdynamic-class-memaccess", memcpy(messageCopy, message, message->GetLength() + 8));
	return messageCopy;
}

uint8* NetClient::AllocateBytesCopy(uint8* bytes, size_t length)
{
	uint8* bytesCopy = (uint8*)NetManager::Inst()->AllocateBytes(length);
	ASSERT(bytesCopy != 0);
	memcpy(bytesCopy, bytes, length);
	return bytesCopy;
}

void NetClient::SendNetMessage(NetMessage& message, bool isReliable)
{
	if (isReliable)
	{
#ifndef FINAL_BUILD
		m_uSentReliable += message.GetLength();
		m_uSentReliableAvg[m_uAvgWindowPos] = message.GetLength();
#endif

		// Set reliable flag
		message.SetReliableIndex(m_nextReliablePacketIndex);
		m_nextReliablePacketIndex++;

		ASSERT_RETURN(!m_reliableMessagesOut.IsFull() && "Reliable messages out is FULL!");
		
		NetMessage* messageCopy = AllocateMessageCopy(&message);
		m_reliableMessagesOut.push(ReliableMessage(messageCopy));
	}
	else
	{
		// Mark as unreliable
		message.SetReliableIndex(-1);

		// Packets were all full and this message was unreliable anyway
		if (m_packetQueueIndex == NET_PACKET_QUEUE_SIZE)
			return;

		// Get a packet of the right length
		NetPacket* packet = &m_packetOutQueue[m_packetQueueIndex];
		packet = VerifyPacketLength(packet, message.GetLength());
		if (packet == 0)
			return;

#ifndef FINAL_BUILD
		m_uSentUnreliable += message.GetLength();
		m_uSentUnreliableAvg[m_uAvgWindowPos] = message.GetLength();
#endif

		// Add to current outgoing packet
		packet->header.dataLength += message.Pack(packet);
		packet->header.numMessages++;
	}
}

void NetClient::OnPacketReceived(NetPacket* packetIn)
{
	// See if any packets need to be acknowledged
	if (packetIn->header.lastReliablePacketRecvd > m_lastAckedPacketIndex)
		m_lastAckedPacketIndex = packetIn->header.lastReliablePacketRecvd;

	// Free any messages in the queue that are hanging out
	while (m_reliableMessagesOut.Count() > 0 &&
			m_reliableMessagesOut.peek().GetReliableIndex() <= m_lastAckedPacketIndex)
	{
		ReliableMessage message = m_reliableMessagesOut.pop();
		message.Free();
	}
	
	// Handle migration, possibly
	CheckIncomingPacketForMigration(packetIn);

	// Read out individual messages that came from this
	sint16 packetOffset = 0;
	int numMessagesRead = 0;
	bool needToAck = false;
	bool endianSwap = (packetIn->header.GetEndianess() != net_get_endian_value());

#ifndef FINAL_BUILD
	m_uReceivedAvg[m_uAvgWindowPos] = 0;
#endif

	while (numMessagesRead < packetIn->header.numMessages)
	{
		//Check if this is a PB message
		sint16 pbLengthHdr = *(sint16*)(packetIn->data + packetOffset);
		bool isPB = pbLengthHdr != 0;

#ifndef FINAL_BUILD
		m_uReceived += packetIn->length;
		m_uReceivedAvg[m_uAvgWindowPos] += packetIn->length;

		if(m_uReceivedAvg[m_uAvgWindowPos] > m_uReceivedPeak)
		{
			m_uReceivedPeak = m_uReceivedAvg[m_uAvgWindowPos];
		}
#endif

		if(isPB)
		{
			numMessagesRead++;

			if(NetManager::Inst()->GetMessageDispatch() == NULL)
			{
				// No active dispatch, so drop the message.
				continue;
			}

			if(endianSwap) { net_endian_swap(pbLengthHdr); }

			ScratchRaw packetBuf(pbLengthHdr, 4);
			packetIn->Unpack(packetBuf.GetRawData(), packetOffset, pbLengthHdr, ProtocolBufferFields<NetMessageHeader>::ID);
			packetOffset += pbLengthHdr;

			ProtocolBufferReader pb((uint8*)packetBuf.GetRawData() + 2, pbLengthHdr - 2);

			NetMessageHeader header;
			bool result = pb.Read(&header);
			ASSERT(result);

			if (header.messageReliableIndex != -1)
			{
				// If we've read this or its out of order, keep reading
				if (header.messageReliableIndex != 1 + m_lastReliableRecvdIndex)
				{
					continue;
				}

				needToAck = true;
				m_lastReliableRecvdIndex = header.messageReliableIndex;
			}

			NetManager::Inst()->GetMessageDispatch()->FireMessage(header.typeIdentifier, pb);
		}
		else
		{
			NetMessage* message = CreateMessageFromBuffer(packetIn, packetOffset, endianSwap);
			packetOffset += message->GetLength();
			numMessagesRead++;

			if (message->GetReliableIndex() != -1)
			{
				// If we've read this or its out of order, keep reading
				if (message->GetReliableIndex() != 1 + m_lastReliableRecvdIndex)
				{
					NetManager::Inst()->FreeMessage(message);
					continue;
				}

				needToAck = true;
				m_lastReliableRecvdIndex = message->GetReliableIndex();
			}
			ReadMessage(message);

			// Free the message
			NetManager::Inst()->FreeMessage(message);
		}
	}

	// If we received a reliable message, ack it
	if (needToAck)
	{
		NetMessagePacketAck packetAckMessage;
		SendNetMessage(packetAckMessage, false);
	}

}

// handle network-level messages here
void NetClient::ReadMessage(NetMessage* message)
{
	// Read and handle incoming messages
	switch (message->GetType())
	{
	case EMT_PING:
	{
		// Update communication time
		m_lastCommunicationTime = net_get_time();

		NetMessagePingClient* pingMessage = (NetMessagePingClient*)message;

		// Send this message back the client
		NetMessagePongClient pongClient;
		pongClient.timeSent = pingMessage->timeSent;

		// Send
		SendNetMessage(pongClient, true);
		break;
	}
	case EMT_PONG:
	{
		// Update communication time
		m_lastCommunicationTime = net_get_time();
		m_nextPingTime = CLIENT_PING_TIME_SECONDS;
		m_bReceivedPong = true;

		NetMessagePongClient* pongMessage = (NetMessagePongClient*)message;

		// Get the current time
		double curTime = net_get_time();
		double timeDif = curTime - pongMessage->timeSent;
		m_pingTimeQueue[m_pingQueueIndex] = timeDif;
		m_pingQueueIndex = (m_pingQueueIndex + 1) % NET_CLIENT_PING_QUEUE_LENGTH;
		m_pingAverage = 0;
		for (int i = 0; i < NET_CLIENT_PING_QUEUE_LENGTH; i++)
		{
			m_pingAverage += m_pingTimeQueue[i];
		}
		m_pingAverage /= NET_CLIENT_PING_QUEUE_LENGTH;

		if (timeDif < m_pingTimeBest)
		{
			m_pingTimeBest = timeDif;
		}
		break;
	}	
	
	case EMT_PACKET_ACK:
	{
		// ACK messages are handled regardless of their content, so
		// no need to perform any handling
		break;
	}

	default:
	{
		// Send to the game because this is a game message
		NetworkGame* game = NetManager::Inst()->GetGame();
		if(game)
		{
			game->OnMessageReceived(message, this);
		}
		else
		{
			DEBUG_PRINT("Dropped message of type %d because there was no valid game\n", message->GetType());
		}
		return;
	}
	}

#ifdef NET_DEBUG_ALLOW_PACKET_INSPECTION
	NetManager::Inst()->DebugMessageTypeReceived(message, 0);
#endif
}

size_t NetClient::WritePBMessageWithHeader(uint8* outputBuf, const size_t outputBufLength, NetMessageHeader& hdr, const uint8* const messageBuf)
{
	ProtocolBufferWriter pb(&outputBuf[2], outputBufLength - 2);

	//Encode header
	bool result = pb.Write(&hdr);
	ASSERT(result);

	//Copy message in
	pb.WriteRaw(messageBuf, hdr.messageLength);

	// Write PB Length header
	sint16* pbLengthHdr = (sint16*)outputBuf;
	*pbLengthHdr = (sint16)pb.GetPos() + 2; // The protocol buffer messages plus 2 bytes for the initial length header

	return *pbLengthHdr;
}

void NetClient::BufferPBMessage(const uint8* messageData, const size_t messageLength, const uint32 messageType, const bool isReliable)
{
	// TODO Could make this ScratchRaw static if we want to avoid allocating scratch
	//      memory every time we send a message, though we would sacrifice thread safety.
	ScratchRaw buf(NETWORK_PACKET_MAX_LENGTH, 4);

	NetMessageHeader hdr;
	hdr.typeIdentifier = messageType;
	hdr.messageLength  = (uint32)messageLength;

	if (isReliable)
	{
		hdr.messageReliableIndex = m_nextReliablePacketIndex;
		m_nextReliablePacketIndex++;

		size_t totalLength = WritePBMessageWithHeader((uint8*)buf.GetRawData(), NETWORK_PACKET_MAX_LENGTH, hdr, messageData);

		ASSERT_RETURN(!m_reliableMessagesOut.IsFull() && "Reliable messages out is FULL!");

		PBMessage pbMsg;
		pbMsg.type = messageType;
		pbMsg.msg = AllocateBytesCopy((uint8*)buf.GetRawData(), totalLength);
		pbMsg.length_bytes = totalLength;
		pbMsg.reliable_index = hdr.messageReliableIndex;

		m_reliableMessagesOut.push(ReliableMessage(pbMsg));
	}
	else
	{
		// Mark as unreliable
		hdr.messageReliableIndex = -1;

		// Packets were all full and this message was unreliable anyway
		if (m_packetQueueIndex == NET_PACKET_QUEUE_SIZE)
			return;

		// Encode message with header
		size_t totalLength = WritePBMessageWithHeader((uint8*)buf.GetRawData(), NETWORK_PACKET_MAX_LENGTH, hdr, messageData);

		// Get a packet of the right length
		NetPacket* packet = &m_packetOutQueue[m_packetQueueIndex];
		packet = VerifyPacketLength(packet, (int)totalLength);
		if (packet == 0)
			return;

		// Add to current outgoing packet
		packet->header.dataLength += packet->Pack((uint8*)buf.GetRawData(), totalLength, hdr.typeIdentifier);
		packet->header.numMessages++;
	}
}

}
