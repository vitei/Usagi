/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemUtil.h"
#include "NetPacket.h"

namespace usg {

NetPacket::NetPacket()
	: m_lastPacketMessageLocation(NULL)
{
}

uint32 NetPacket::Pack(void* data_in, size_t data_length, uint32 type)
{
	uint8* start = (uint8*)data_in;

	// For now, disable packing on PB messages (just copy directly)
	// dpw 20150731 -- Actually for now disable on ALL messages
//	if(start[0] != 0 || start[1] != 0)
	{
		MemCpy(&data[header.dataLength], start, data_length);
		return (uint32)data_length;
	}
/*
	// See if the last message type matched
	if (header.numMessages > 0 &&
	    m_lastPackedMessageType == type &&
	    m_lastPacketMessageLocation != NULL)
	{
		// XOR pack

		// Get the PB-ness
		data[header.dataLength + 0] = start[0];
		data[header.dataLength + 1] = start[1];

		// Set the packet type to 0x0
		data[header.dataLength + 2] = 0x0;
		data[header.dataLength + 3] = 0x0;

		// Get the size
		data[header.dataLength + 4] = start[4];
		data[header.dataLength + 5] = start[5];

		// XOR from last packet
		for (size_t i = 6; i < data_length; i++)
		{
			data[header.dataLength + i] = start[i] ^ m_lastPacketMessageLocation[i];
		}
	}
	else
	{
		// Standard pack
		for (size_t i = 0; i < data_length; i++)
		{
			data[header.dataLength + i] = start[i];
		}

		// We updated the type, so save it
		m_lastPackedMessageType = type;
		m_lastPacketMessageLocation = data + header.dataLength;
	}


	return data_length;
*/
}

void NetPacket::Unpack(void* data_out, sint16 offset, sint16 data_length, uint32 type)
{
	uint8* start = (uint8*)data_out;

	// For now, packing is disabled on PB messages (just copy directly)
	// dpw 20150731 -- Actually for now disable on ALL messages
//	if(data[offset + 0] != 0 || data[offset + 1] != 0)
	{
		MemCpy(data_out, &data[offset], data_length);
		return;
	}

/*
	// XOR packet
	if (data[offset + 2] == 0 &&
	    data[offset + 3] == 0)
	{

		// Get the actual type
		for (sint16 j = 0; j < 6; j++)
		{
			start[j] = m_lastPacketMessageLocation[j];
		}

		// Get out the XOR data
		for (sint16 i = 6; i < data_length; i++)
		{
			start[i] = data[offset + i] ^ m_lastPacketMessageLocation[i];
		}
	}
	else
	{
		for (sint16 i = 0; i < data_length; i++)
		{
			start[i] = data[offset + i];
		}

		// Official packet we are XORing from
		m_lastPackedMessageType = type;
		m_lastPacketMessageLocation = (data + offset);
	}
*/
}


}

