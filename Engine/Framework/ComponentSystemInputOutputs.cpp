/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Framework/ComponentGetter.h"
#include "Engine/Framework/ComponentSystemInputOutputs.h"
#include "Engine/Core/stl/hash_map.h"
#include "Engine/Memory/UnTypesafeFastPool.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/stl/memory.h"

namespace usg
{
	static usg::unique_ptr<hash_map<uint32, usg::unique_ptr<UnTypesafeFastPool>>> s_pPools;
	static usg::unique_ptr<usg::vector<GenericInputOutputs*>> s_pRootNodes;

	void ComponentSystemInputOutputsSharedBase::InitBase(uint32 uSystemId, uint32 uElementSize, uint32 uGroupSize, GenericInputOutputs* pRootNode)
	{
		if (!s_pPools)
		{
			s_pPools.reset(vnew(ALLOC_OBJECT) hash_map<uint32, usg::unique_ptr<UnTypesafeFastPool>>);
		}
		auto& pools = *s_pPools;
		ASSERT(pools.count(uSystemId)==0);
		pools[uSystemId].reset(vnew(ALLOC_OBJECT) UnTypesafeFastPool(uElementSize,uGroupSize));
		
		if (!s_pRootNodes)
		{
			s_pRootNodes.reset(vnew(ALLOC_OBJECT) usg::vector<GenericInputOutputs*>());
		}
		if (uSystemId >= s_pRootNodes->size())
		{
			s_pRootNodes->resize(uSystemId + 64);
		}
		(*s_pRootNodes)[uSystemId] = pRootNode;
	}

	void ComponentSystemInputOutputsSharedBase::Cleanup()
	{
		s_pPools.reset(nullptr);
		s_pRootNodes.reset(nullptr);
	}

	void ComponentSystemInputOutputsSharedBase::CleanupBase(uint32 uSystemId)
	{
		ASSERT(s_pPools != nullptr);
		auto& pools = *s_pPools;
		ASSERT(pools.count(uSystemId) == 1);
		pools[uSystemId].reset(nullptr);
		pools.erase(uSystemId);
		if (pools.size() == 0)
		{
			Cleanup();
		}
	}

	void* ComponentSystemInputOutputsSharedBase::Alloc(uint32 uSystemId)
	{
		ASSERT(s_pPools != nullptr);
		auto& pools = *s_pPools;
		ASSERT(pools.count(uSystemId) == 1);
		void* pAllocated = pools[uSystemId]->Alloc();
		return pAllocated;
	}

	void ComponentSystemInputOutputsSharedBase::Free(uint32 uSystemId, void* pIO)
	{
		ASSERT(s_pPools != nullptr);
		auto& pools = *s_pPools;
		ASSERT(pools.count(uSystemId) == 1);

		GenericInputOutputs* pTest = (GenericInputOutputs*)pIO;
		pools[uSystemId]->Free(pIO);
	}

	GenericInputOutputs* ComponentSystemInputOutputsSharedBase::GetRootSystem(uint32 uSystemId)
	{
		ASSERT(s_pRootNodes != nullptr);
		ASSERT(uSystemId < s_pRootNodes->size());
		return (*s_pRootNodes)[uSystemId];
	}

	void ComponentSystemInputOutputsSharedBase::RemoveGenericInputOutputs(Entity e, uint32 uSystemId)
	{
		if (e->GetSystem(uSystemId) != NULL)
		{
			e->GetSystem(uSystemId)->DetachSelf();
			Free(uSystemId, e->GetSystem(uSystemId));
			e->SetSystem(uSystemId, NULL);
		}
	}

	void ComponentSystemInputOutputsSharedBase::AttachGenericInputOutputs(Entity e, GenericInputOutputs* pIO, GenericInputOutputs* pRoot, uint32 uSystemId)
	{
		Entity attachEntity = e->GetParentEntity();
		GenericInputOutputs* pAttachIO = NULL;
		while (attachEntity)
		{
			pAttachIO = attachEntity->GetSystem(uSystemId);
			if (pAttachIO)
			{
				break;
			}
			attachEntity = attachEntity->GetParentEntity();
		}

		if (pAttachIO)
		{
			pIO->AttachToNode(pAttachIO);
		}
		else
		{
			pIO->AttachToNode(pRoot);
		}
		pIO->entity = e;
		e->SetSystem(uSystemId, pIO);
	}
}