#include "Engine/Common/Common.h"
#include "GameComponents.h"
#include "Engine/Framework/ComponentEntity.h"
#include "Engine/Core/stl/memory.h"

namespace usg
{

	void ComponentType::Activate(Entity e)
	{
		Init();
		m_uEntity = e;
		const uint32 uTypeId = GetTypeID();
		e->SetComponent(uTypeId, this);
		ComponentStats::ActivatedComponent(uTypeId);
	}

	void* ComponentGetterBase::GetComponentBase(Entity e, uint32 uComponentId, sint32 iSearchMask)
	{
		void* pComponent = nullptr;
		Entity uCheckEntity = (iSearchMask & FROM_SELF) ? e : e->GetParentEntity();

		pComponent = uCheckEntity != nullptr ? uCheckEntity->GetComponent(uComponentId) : nullptr;
		if ((pComponent == nullptr) && (iSearchMask & FROM_PARENTS))
		{
			uCheckEntity = uCheckEntity->GetParentEntity();
			while (!pComponent && uCheckEntity != NULL)
			{
				pComponent = uCheckEntity != nullptr ? uCheckEntity->GetComponent(uComponentId) : nullptr;
				uCheckEntity = uCheckEntity->GetParentEntity();
			}
		}
		
		return pComponent;
	}

	bool GetComponent(Entity e, uint32 uComponentId, sint32 iSearchMask, ComponentWrapperBase& wrapper)
	{
		Entity uCheckEntity = (iSearchMask & FROM_SELF) ? e : e->GetParentEntity();
		ComponentType* pComponent = uCheckEntity != nullptr ? uCheckEntity->GetComponent(uComponentId) : nullptr;
		if ((pComponent == nullptr) && (iSearchMask & FROM_PARENTS))
		{
			uCheckEntity = uCheckEntity->GetParentEntity();
			while (!pComponent && uCheckEntity != nullptr)
			{
				pComponent = uCheckEntity != nullptr ? uCheckEntity->GetComponent(uComponentId) : nullptr;
				uCheckEntity = uCheckEntity->GetParentEntity();
			}
		}
		wrapper = ComponentWrapperBase(pComponent, USG_INVALID_ID);
		return pComponent != nullptr;
	}

	
	static usg::unique_ptr<UnTypesafeFastPool> s_pComponentFastPools[ComponentEntity::MAX_COMPONENT_TYPES];

	static struct ComponentFunctionTable
	{
		void(*pActivate)(ComponentType*,Entity) = nullptr;
		void(*pDeactivate)(ComponentType*, ComponentLoadHandles& handles, bool) = nullptr;
	} s_componentFunctionTables[ComponentEntity::MAX_COMPONENT_TYPES];

	void GameComponentMgr::Cleanup(ComponentLoadHandles& handles)
	{
		for (uint32 uTypeId = 0;uTypeId < ARRAY_SIZE(s_pComponentFastPools);uTypeId++)
		{
			auto pPool = GetPool(uTypeId);
			if (pPool != nullptr)
			{
				for (auto it = pPool->_BeginDynamic<ComponentType>(); !it.IsEnd(); ++it)
				{
					ComponentType* cmp = *it;
					if (cmp->GetEntity() && cmp->GetEntity()->HasComponent(uTypeId))
					{
						Free(cmp, handles, true);
					}
				}
				FreePool(uTypeId);
			}
		}
	}

	ComponentType* GameComponentMgr::Create(uint32 uTypeId, Entity e)
	{
		if (!GetPool(uTypeId))
		{
			DEBUG_PRINT("WARNING: Creating a Component (%u) not used by any System - please register manually in ComponentManager.\n", uTypeId);
			ASSERT(false);
			return nullptr;
		}
		ComponentType* c = AllocFromPool(uTypeId);
		c->SetNextComponent(NULL);
		c->SetPrevComponent(NULL);
		ASSERT(c != NULL);

		s_componentFunctionTables[uTypeId].pActivate(c, e);
		return c;
	}
	
	void GameComponentMgr::FreePool(uint32 uTypeId)
	{
		s_pComponentFastPools[uTypeId].reset(nullptr);
		uint32 uCount = 0;
		for (auto& p : s_pComponentFastPools) {
			if (p != nullptr) {
				uCount++;
			}
		}
		if (uCount == 0)
		{
			DEBUG_PRINT("ComponentMgr: All component fast pools cleaned up.\n");
		}
	}

	void GameComponentMgr::Free(ComponentType* pComponent, ComponentLoadHandles& handles, bool bRemoveFromEntity)
	{
		if (pComponent)
		{
			const uint32 uTypeId = pComponent->GetTypeID();
			auto pPool = GetPool(uTypeId);
			ASSERT(pPool != nullptr);
			s_componentFunctionTables[uTypeId].pDeactivate(pComponent, handles, bRemoveFromEntity);
			pPool->Free(pComponent);
		}
	}

	bool GameComponentMgr::IsInitialized(uint32 uTypeId)
	{
		if (uTypeId >= ARRAY_SIZE(s_pComponentFastPools))
		{
			return false;
		}
		return s_pComponentFastPools[uTypeId] != nullptr;
	}

	void GameComponentMgr::InitPool(uint32 uTypeId, memsize uElementSize, uint32 uPoolChunkSize, void(*pComponentConstructorCaller)(void*), void(*pComponentDestructorCaller)(void*), void(*pActivate)(ComponentType*,Entity), void(*pDeactivate)(ComponentType*, ComponentLoadHandles& handles, bool))
	{
		if (s_pComponentFastPools[uTypeId])
		{
			return;
		}
		auto pPool = vnew(ALLOC_COMPONENT) UnTypesafeFastPool(uElementSize, uPoolChunkSize, true, false, pComponentConstructorCaller, pComponentDestructorCaller);
		s_pComponentFastPools[uTypeId].reset(pPool);

		auto& functionTable = s_componentFunctionTables[uTypeId];
		functionTable.pActivate = pActivate;
		functionTable.pDeactivate = pDeactivate;
	}

	UnTypesafeFastPool* GameComponentMgr::GetPool(uint32 uTypeId)
	{
		return s_pComponentFastPools[uTypeId].get();
	}

	ComponentType* GameComponentMgr::AllocFromPool(uint32 uTypeId)
	{
		auto pPool = s_pComponentFastPools[uTypeId].get();
		ASSERT(pPool != nullptr);
		return (ComponentType*)pPool->Alloc();
	}
}