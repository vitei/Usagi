/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef NET_AVAILABLE_GAMES_H
#define NET_AVAILABLE_GAMES_H

namespace usg {
struct NetPacket;
}

#include "NetMessage.h"

namespace usg
{

struct NetAvailableGame
{
	// Game ID
	sint64 gameUID;
	sint32 gameAvailable;

	// Connected Clients (players)
	NetSimplePlayerData players[USAGI_NET_MAX_CLIENTS];

	// Age
	double discoveredTime;

	void UpdateData(NetMessageGameAvailable* netMessage);
};

class AvailableGameInfoContainer
{
public:
	static const uint32 InvalidId = (uint32)(-1);
	static const uint32 MaxGameCount = 10;
	static const uint32 MaxApplicationDataSize = 256;

	struct GameInfo {
		uint32 uId;
		uint32 uAppDataSize;
		double fTimeStamp;
		uint8 uAppData[MaxApplicationDataSize];
		uint32 uCurrentParticipantCount;
		uint32 uMaxParticipantCount;
	};
private:

	GameInfo m_game[MaxGameCount];

	uint32 FindFreeIndex() const;
	uint32 GetGameIndex(uint32 uId) const;

public:

	struct Iterator
	{
		const AvailableGameInfoContainer* pContainer;
		uint32 uIndex;
	public:
		Iterator(const AvailableGameInfoContainer& container, uint32 uIndex) : pContainer(&container), uIndex(uIndex)
		{
			if (pContainer->m_game[uIndex].uId == InvalidId)
			{
				++(*this);
			}
		}

		Iterator& operator++()
		{
			if (*this != pContainer->End())
			{
				uIndex++;
				while (pContainer->m_game[uIndex].uId == InvalidId && *this != pContainer->End())
				{
					uIndex++;
				}
			}
			return *this;
		}

		Iterator& operator++(int)
		{
			++(*this);
			return *this;
		}

		bool operator!=(const Iterator& rhs)
		{
			return !(pContainer == rhs.pContainer && uIndex == rhs.uIndex);
		}

		bool operator==(const Iterator& rhs)
		{
			return (pContainer == rhs.pContainer && uIndex == rhs.uIndex);
		}

		const AvailableGameInfoContainer::GameInfo& operator*()
		{
			return pContainer->m_game[uIndex];
		}
	};

	AvailableGameInfoContainer();
	Iterator Begin() const;
	Iterator End() const;
	void Clear();
	bool IsFull() const;
	bool Contains(uint32 uId) const;
	bool Remove(uint32 uId);
	const GameInfo& operator[](uint32 uId) const;
	bool Add(uint32 uId, uint32 uCurrentParticipants, uint32 uMaxParticipants, double fTimeStamp, const void* pApplicationData, uint32 uApplicationDataSize);
};

typedef AvailableGameInfoContainer::Iterator AvailableGameIterator;

}

#endif
