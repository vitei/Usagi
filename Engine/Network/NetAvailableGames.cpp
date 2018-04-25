/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemUtil.h"
#include "NetAvailableGames.h"
#include "NetMessage.h"
#include "NetPlatform.h"
#include "NetPacket.h"

namespace usg
{

	uint32 AvailableGameInfoContainer::FindFreeIndex() const
	{
		for (uint32 uIndex = 0; uIndex < MaxGameCount; uIndex++)
		{
			if (m_game[uIndex].uId == InvalidId)
			{
				return uIndex;
			}
		}
		return InvalidId;
	}

	uint32 AvailableGameInfoContainer::GetGameIndex(uint32 uId) const
	{
		for (uint32 uIndex = 0; uIndex < MaxGameCount; uIndex++)
		{
			if (m_game[uIndex].uId == uId)
			{
				return uIndex;
			}
		}
		return InvalidId;
	}

	AvailableGameInfoContainer::AvailableGameInfoContainer() {
		Clear();
	}

	AvailableGameInfoContainer::Iterator AvailableGameInfoContainer::Begin() const
	{
		return Iterator(*this, 0);
	}

	AvailableGameInfoContainer::Iterator AvailableGameInfoContainer::End() const
	{
		return AvailableGameInfoContainer::Iterator(*this, MaxGameCount);
	}

	void AvailableGameInfoContainer::Clear()
	{
		for (uint32 uIndex = 0; uIndex < MaxGameCount; uIndex++)
		{
			m_game[uIndex].uId = InvalidId;
		}
	}

	bool AvailableGameInfoContainer::IsFull() const
	{
		if (FindFreeIndex() != InvalidId)
		{
			return false;
		}
		return true;
	}

	bool AvailableGameInfoContainer::Contains(uint32 uId) const
	{
		return GetGameIndex(uId) != InvalidId;
	}

	bool AvailableGameInfoContainer::Remove(uint32 uId)
	{
		const uint32 uIndex = GetGameIndex(uId);
		if (uIndex != InvalidId)
		{
			m_game[uIndex].uId = InvalidId;
			return true;
		}
		return false;
	}

	const AvailableGameInfoContainer::GameInfo& AvailableGameInfoContainer::operator[](uint32 uId) const
	{
		const uint32 uIndex = GetGameIndex(uId);
		ASSERT(uIndex != InvalidId);
		return m_game[uIndex];
	}

	bool AvailableGameInfoContainer::Add(uint32 uId, uint32 uCurrentParticipants, uint32 uMaxParticipants, double fTimeStamp, const void* pApplicationData, uint32 uApplicationDataSize)
	{
		const bool bDidContain = Contains(uId);
		const uint32 uIndex = bDidContain ? GetGameIndex(uId) : FindFreeIndex();
		if (uIndex == InvalidId)
		{
			return false;
		}

		ASSERT(uApplicationDataSize <= MaxApplicationDataSize);
		if (uApplicationDataSize > 0)
		{
			MemCpy(m_game[uIndex].uAppData, pApplicationData, uApplicationDataSize);
			m_game[uIndex].uAppDataSize = uApplicationDataSize;
		}
		m_game[uIndex].uId = uId;
		m_game[uIndex].fTimeStamp = fTimeStamp;
		m_game[uIndex].uCurrentParticipantCount = uCurrentParticipants;
		m_game[uIndex].uMaxParticipantCount = uMaxParticipants;
		return !bDidContain;
	}

void NetAvailableGame::UpdateData(NetMessageGameAvailable* netMessage)
{
	// Copy the player data over
	MemCpy(players, netMessage->client, sizeof(NetSimplePlayerData)* USAGI_NET_MAX_CLIENTS);

	// Game UID
	gameUID = netMessage->gameUID;

	// Register the time
	discoveredTime = net_get_time();
}

}

