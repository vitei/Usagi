/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "CatchupEntities.h"

using namespace usg;

CatchupEntities::CatchupEntities()
	: m_pMem(nullptr)
	, m_uQueueSize(0)
{
	
}

CatchupEntities::~CatchupEntities()
{
	
}

void CatchupEntities::Clear()
{
	m_uQueueSize = 0;
}

void CatchupEntities::RegisterEntity(Entity e, double t)
{
	ASSERT(m_uQueueSize < MaxCatchupEntities);
	CatchupEntity& pair = m_catchupQueue[m_uQueueSize++];
	pair.first = e;
	pair.second = t;
}

CatchupEntities::CatchupEntity CatchupEntities::Pop()
{
	ASSERT(m_uQueueSize > 0);
	return m_catchupQueue[--m_uQueueSize];
}

uint32 CatchupEntities::NumEntities() const
{
	return m_uQueueSize;
}
