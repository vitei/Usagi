/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// CatchupEntities
// Tracks entities which have been spawned in the past (perhaps over the network)
// and need to catch up.

#pragma once

#ifndef USAGI_FRAMEWORK_CATCHUP_ENTITIES_H
#define USAGI_FRAMEWORK_CATCHUP_ENTITIES_H

#include "Engine/Memory/MemHeap.h"
#include "Engine/Memory/FastPool.h"
#include "Engine/Framework/ComponentEntity.h"

#include "Engine/Core/stl/utility.h"

namespace usg {

class CatchupEntities
{
public:
	typedef usg::pair<Entity, double> CatchupEntity;

	CatchupEntities();
	~CatchupEntities();

	void Clear();
	void RegisterEntity(Entity e, double t);
	CatchupEntity Pop();
	uint32 NumEntities() const;

private:
	static const uint32 MaxCatchupEntities = 64;

	void* m_pMem;
	uint32 m_uQueueSize;
	CatchupEntity m_catchupQueue[MaxCatchupEntities];
};

} // namespace usg

#endif //USAGI_FRAMEWORK_CATCHUP_ENTITIES_H
