/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "NetPacket.h"
#include "NetMessage.h"
#include "Engine/Core/Containers/Queue.h"
#include "Engine/Common/Common.h"
#include "Engine/Framework/MessageWithHeader.h"

#ifndef FINAL_BUILD
static const int WINDOW_LENGTH = 5;
#endif

namespace usg
{

// Forward Dec
struct NetMessage;

// Reliable messages can either be NetMessages or PB Messages
struct PBMessage
{
	void*  msg;
	uint32 type;
	size_t length_bytes;
	int    reliable_index;
};

struct ReliableMessage
{
	ReliableMessage();
	ReliableMessage(NetMessage* msg);
	ReliableMessage(const PBMessage& msg);

	int GetReliableIndex() const;
	void Free();
	sint16 GetLength() const;
	uint32 Pack(struct NetPacket* packet);

private:
	bool isPB;
	union
	{
		NetMessage* netMessage;
		PBMessage   pbMessage;
	};
};

// Number of reliable messages we reasonably have out at one time
#define RELIABLE_MESSAGE_QUEUE_LENGTH	(128)
typedef Queue<ReliableMessage, RELIABLE_MESSAGE_QUEUE_LENGTH> NetMessageQueue;

#define NET_CLIENT_PING_QUEUE_LENGTH	(8)
#define NET_DISCONNECTED_PING_TIME		(100000.0f)
#define NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH (4)
#define NET_PACKET_QUEUE_SIZE			(4)

#define CLIENT_PACKET_TTS_SECONDS		(.032f)
#define CLIENT_PING_TIME_SECONDS		(.65f)

// Data that holds all information to send and receive from an endpoint
class NetClient
{
public:
	NetClient();
	virtual ~NetClient();

	// main functions
	virtual void Disconnect();
	void Update(float deltaTime);

	// Packet functions
	void SendNetMessage(NetMessage& message, bool isReliable);
	void OnPacketReceived(NetPacket* inPacket);

	// PB versions
	template<typename T>
	void SendPBMessage(T& message, bool isReliable);

	// Identity
	void SetColor(uint8 color){ m_color = color; }
	void SetName(char* name);
	char* GetName() { return m_name; }
	int GetPlayerID() { return m_playerIndex; }
	virtual sint64 GetUID() = 0;
	uint8 GetColor() { return m_color;  }


	// Maintenance
	void SetActive(bool active);
	bool GetIsActive() const { return m_isActive; }
	double GetTimeSinceLastCommunication();
	double GetPing() { return m_pingAverage; }
	float GetAverageBandwidth();
	float GetBandwidthCompressionRate();
	sint16 GetNumUnackedMessages() const { return m_reliableMessagesOut.Count(); }
	bool HasReceivedClientReport() const
	{
		return m_bReceivedClientReport;
	}

	float32 GetActivationTime() const
	{
		return m_fActivationTime;
	}

#ifndef FINAL_BUILD
	void ResetLengthSentReceived();
	uint32 GetLengthSent(bool bReliable) { return (bReliable ? m_uSentReliable : m_uSentUnreliable); }
	uint32 GetLengthReceived() { return m_uReceived; }
	float GetAvg(uint32* pArray, uint32 uLen);
	float GetAvgLengthSent(bool bReliable);
	float GetAvgLengthReceived() { return GetAvg(m_uReceivedAvg, WINDOW_LENGTH); }
	uint32 GetPeakLengthReceived() { return m_uReceivedPeak; }
	uint32 GetLastLengthReceived() { return m_uReceivedAvg[m_uAvgWindowPos]; }
#endif

protected:

	// Utilities
	virtual bool CheckForTimeout();
	void UpdatePing(float deltaTime);
	void ResetTimeToSend() { m_timeToSend = CLIENT_PACKET_TTS_SECONDS;  }
	void AddToBandwidth(sint64 usedBandwidth) { m_totalBytesOut += usedBandwidth;}
	virtual void CheckIncomingPacketForMigration(NetPacket* packetIn) = 0;
	virtual void SetOutgoingPacketHeader(NetPacket* packet) = 0;
	virtual void SendPacketToClient();
	virtual void SendPacketToMulti() = 0;
	virtual void ReadMessage(NetMessage* message);
	void SetPlayerIndex(int p) { m_playerIndex = p;  }

	// Packet Management (internal to children classes)
	NetPacket m_packetOutQueue[NET_PACKET_QUEUE_SIZE];
	int m_packetQueueIndex;
	bool m_bReceivedClientReport;
	float m_fActivationTime;
private:

	// Time data
	double m_lastCommunicationTime;
	float m_timeToSend;


	// Ping data
	double m_pingTimeBest;
	double m_pingAverage;
	double m_pingTimeQueue[NET_CLIENT_PING_QUEUE_LENGTH];
	int m_pingQueueIndex;
	float m_nextPingTime;
	bool m_bReceivedPong;

	// Identification
	char m_name[USAGI_NET_CLIENT_MAX_NAME_LENGTH];
	bool m_isActive;
	int m_playerIndex;
	uint8 m_color;

	// Traffic
	sint64 m_totalBytesOut;
	sint64 m_bandwidthCounter[NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH+1];
	sint64 m_origBandwithCounter[NET_CLIENT_BANDWIDTH_COUNTER_QUEUE_LENGTH + 1];
	int m_bandwidthIndex;
	float m_bandwidthTimer;

	// Incoming
	int m_lastAckedPacketIndex;
	int m_lastReliableRecvdIndex;

	// Outgoing 
	int m_nextReliablePacketIndex;
	NetMessageQueue m_reliableMessagesOut;

#ifndef FINAL_BUILD
	uint32 m_uSentReliable;
	uint32 m_uSentUnreliable;
	uint32 m_uReceived;
	uint32 m_uSentReliableAvg[WINDOW_LENGTH];
	uint32 m_uSentUnreliableAvg[WINDOW_LENGTH];
	uint32 m_uReceivedAvg[WINDOW_LENGTH];
	uint32 m_uReceivedPeak;
	uint32 m_uAvgWindowPos;
#endif

	// Utilities
	void PackReliableMessages();
	NetPacket* VerifyPacketLength(NetPacket* packet, int length);
	NetMessage* AllocateMessageCopy(NetMessage* message);
	uint8* AllocateBytesCopy(uint8* bytes, size_t length);
	void CalculateBandwidth(float deltaTime);

	size_t WritePBMessageWithHeader(uint8* outputBuf, const size_t outputBufLength, NetMessageHeader& hdr, const uint8* const messageBuf);
	void BufferPBMessage(const uint8* messageData, const size_t messageLength, const uint32 messageType, const bool isReliable);

	template<typename T> friend struct PBMessageSender;
	template<typename T>
	struct PBMessageSender
	{
		NetClient* self;
		PBMessageSender(NetClient* _self) : self(_self) {}

		void operator()(T& message, bool isReliable)
		{
			// TODO Could make this ScratchRaw static if we want to avoid allocating scratch
			//      memory every time we send a message, though we would sacrifice thread safety.
			ScratchRaw msgBuf(NETWORK_PACKET_MAX_LENGTH, 4);
			ProtocolBufferWriter pb((uint8*)msgBuf.GetRawData(), NETWORK_PACKET_MAX_LENGTH);

			bool result = pb.Write(&message);
			ASSERT(result);

			size_t msgLength = pb.GetPos();
			self->BufferPBMessage((uint8*)msgBuf.GetRawData(), msgLength, ProtocolBufferFields<T>::ID, isReliable);
		}
	};

	template<typename HeaderType, typename PayloadType>
	struct PBMessageSender< MessageWithHeader<HeaderType, PayloadType> >
	{
		NetClient* self;
		PBMessageSender(NetClient* _self) : self(_self) {}

		void operator()(MessageWithHeader<HeaderType, PayloadType>& message, bool isReliable)
		{
			ScratchRaw msgBuf(NETWORK_PACKET_MAX_LENGTH, 4);
			ProtocolBufferWriter pb((uint8*)msgBuf.GetRawData(), NETWORK_PACKET_MAX_LENGTH);

			bool result;
			result = pb.Write(&message.hdr);
			ASSERT(result);
			result = pb.Write(&message.body);
			ASSERT(result);

			size_t msgLength = pb.GetPos();
			self->BufferPBMessage((uint8*)msgBuf.GetRawData(), msgLength, ProtocolBufferFields< MessageWithHeader<HeaderType, PayloadType> >::ID, isReliable);
		}
	};
};

template<typename T>
void NetClient::SendPBMessage(T& message, bool isReliable)
{
	PBMessageSender<T> send(this);
	send(message, isReliable);
}

}

#endif
