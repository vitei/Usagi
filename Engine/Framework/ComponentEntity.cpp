/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "ComponentEntity.h"
#include "GameComponents.h"
#include "ComponentStats.h"
#include "NewEntities.h"
#include "Engine/Framework/SystemCoordinator.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Core/stl/memory.h"
#include "Engine/Framework/NewEntities.h"

namespace usg {

	template<> void RegisterComponent<usg::Components::EntityID>(SystemCoordinator& systemCoordinator) { systemCoordinator.RegisterComponent<::usg::Components::EntityID>(); }

	static unique_ptr<NewEntities> s_pNewEntities;

	NewEntities& ComponentEntity::GetNewEntities()
	{
		return *s_pNewEntities;
	}

static uint32 s_uEntityNum=1;
uint32 ComponentEntity::g_uNumSystemTypes=0;
ComponentEntity	*		ComponentEntity::g_hierarchy;
FastPool< ComponentEntity >* ComponentEntity::g_pPool = nullptr;
void* g_mainMem = nullptr;
MemHeap* g_pMemHeap = nullptr;

void ComponentEntity::InitPool(uint32 uPoolSize)
{
	static const size_t alloc_size = 1024 * 2048;
	g_pPool = vnew(ALLOC_COMPONENT) FastPool< ComponentEntity >(uPoolSize, true, false);
	g_mainMem = mem::Alloc(MEMTYPE_STANDARD, ALLOC_OBJECT, alloc_size);
	g_pMemHeap = vnew(ALLOC_COMPONENT) MemHeap;
	g_pMemHeap->Initialize(g_mainMem, alloc_size);
	g_hierarchy = g_pPool->Alloc();
	g_hierarchy->Activate();
	s_pNewEntities.reset(vnew(ALLOC_OBJECT)NewEntities());
}

StringPointerHash<GenericInputOutputs*>& ComponentEntity::GetSystems()
{
	return m_pSystems;
}

void ComponentEntity::Reset()
{
	if (g_pPool)
	{
		vdelete g_pPool;
		g_pPool = nullptr;

		vdelete(g_pMemHeap);
		g_pMemHeap = nullptr;

		mem::Free(g_mainMem);
		g_mainMem = nullptr;
	}
	s_pNewEntities.reset(nullptr);
	g_uNumSystemTypes = 0;
	s_uEntityNum = 1;
}

ComponentEntity::ComponentEntity() : m_pComponents(StringPointerHash<ComponentType*>(12)), m_pSystems(StringPointerHash<GenericInputOutputs*>(12))
{
	m_bActive = false;
	m_uIndex = s_uEntityNum++;
	m_fCatchupTime = 0.0f;
	m_uOnCollisionMask = 0;
}

ComponentEntity::~ComponentEntity()
{

}

void ComponentEntity::SetComponent(uint32 uCompIndex, ComponentType *pComp)
{
	uint32 uCompID = GetComponentIDFromIndex(uCompIndex);
	uint32 bitfieldOffset = uCompIndex / BITFIELD_LENGTH;
	uint32 bitfieldIndex = uCompIndex % BITFIELD_LENGTH;
	if (pComp!=nullptr)
	{
		ASSERT(!m_pComponents.Exists(uCompID));
		m_pComponents.Insert(uCompID, pComp);
		LinkComponent(pComp);
	}
	else
	{
		ComponentType* value = m_pComponents.Delete(uCompID);
		if(value != nullptr)
		{
			ASSERT(value->GetTypeID() == uCompIndex);
			UnlinkComponent(value);
		}
	}

	SetComponentBit(bitfieldOffset, bitfieldIndex, pComp != nullptr);
	SetChanged();
}

void ComponentEntity::Activate()
{
	ASSERT(!m_bActive);
    
	m_pComponents.SetAllocator(g_pMemHeap);
	m_pSystems.SetAllocator(g_pMemHeap);
	m_pComponents.Clear();
	m_pSystems.Clear();

	m_pFirstComponent = nullptr;

	for (uint32 i = 0; i < (MAX_COMPONENT_TYPES / BITFIELD_LENGTH) + 1; i++)
	{
		m_uComponentBitfield[i] = 0;
	}
    
	m_bChanged = false;
	m_bChildrenChanged = false;
   
    
	m_bActive = true;
	m_fCatchupTime = 0.0f;
    
	ComponentStats::ActivatedEntity();
    
}

uint32 ComponentEntity::NextSystemType()
{
	return g_uNumSystemTypes++;
}

void ComponentEntity::SetParent(ComponentEntity* pParent)
{
	DetachSelf();
	AttachToNode(pParent);
	SetChanged();
}

void ComponentEntity::SetSystem(uint32 uSysIndex, GenericInputOutputs *pSys)
{
	uint32 uSysID = GetSystemIDFromIndex(uSysIndex);
	if(pSys == nullptr)
	{
		if(m_pSystems.Exists(uSysID))
		{
			m_pSystems.Delete(uSysID);
		}
	}
	else
	{
		ASSERT(!m_pSystems.Exists(uSysID));
		m_pSystems.Insert(uSysID, pSys);
	}
	SetChanged();
}

GenericInputOutputs* ComponentEntity::GetSystem(uint32 uSysIndex) const
{
	return m_pSystems.Get(GetSystemIDFromIndex(uSysIndex));
}

void ComponentEntity::SetComponentPendingDelete()
{
	m_bPendingDeletions = true;
	SetChanged();
}

void ComponentEntity::SetChanged()
{
	m_bChanged = true;
	ComponentEntity* parent = GetParentEntity();
	if(parent != nullptr)
		parent->SetChildrenChanged();
}


void ComponentEntity::HandlePendingDeletes(ComponentLoadHandles& handles)
{
	if (!m_bPendingDeletions)
		return;

	ComponentType* pComponent = m_pFirstComponent;
	while (pComponent)
	{
		// Cache the next pointer as free could remove it from the link
		ComponentType* pNextComponent = pComponent->GetNextComponent();
		if (pComponent->WaitingOnFree())
		{
			pComponent->Free(handles, true);
		}
		pComponent = pNextComponent;
	}

	m_bPendingDeletions = false;
}

void ComponentEntity::SetChildrenChanged()
{
	m_bChildrenChanged = true;
	ComponentEntity* parent = GetParentEntity();
	if(parent != nullptr)
		parent->SetChildrenChanged();
}

void ComponentEntity::Deactivate()
{
	ASSERT(m_bActive);
    
	m_bActive = false;
    
	ComponentStats::DeactivatedEntity();
}

ComponentEntity* ComponentEntity::GetChildEntityByName(const UnsafeComponentGetter& getter, uint32 uNameHash, bool bRecursive)
{
	ComponentEntity* pChild = GetChildEntity();
	while (pChild)
	{
		// skip any entities flagged as dead
		Optional<StateComponent> state;
		getter.GetComponent(pChild, state);
		if (state.Exists() && state.Force()->current == STATUS_DEAD)
		{
			continue;
		}

		Optional<Identifier> id;
		getter.GetComponent(pChild, id);
		if (id.Exists() && id.Force().GetRuntimeData().uNameHash == uNameHash)
		{
			return pChild;
		}

		if (bRecursive)
		{
			ComponentEntity* pRecursive = pChild->GetChildEntityByName(getter, uNameHash, true);
			if (pRecursive)
			{
				return pRecursive;
			}
		}
		pChild = pChild->m_pNextSibling;
	}
	return nullptr;
}

ComponentEntity* ComponentEntity::GetChildEntityByName(const UnsafeComponentGetter& getter, const char* szName, bool bRecursive)
{
	return GetChildEntityByName(getter, utl::CRC32(szName), bRecursive);
}

ComponentEntity* ComponentEntity::Create(ComponentEntity* parent)
{
	ASSERT(g_pPool!=nullptr);
    
	ComponentEntity * e = g_pPool->Alloc();
	ASSERT(e->m_pSystems.IsEmpty());
	e->Activate();

	auto& ne = GetNewEntities();
	ne.Add(e,parent);

	if (parent != nullptr && ne.Contains(parent))
	{
		e->AttachToNode(parent);
	}

	EntityID* entity = GameComponents<EntityID>::Create(e);
	entity->id = e;
    
	return e;
}

void ComponentEntity::Free(Entity e, ComponentLoadHandles& handles)
{
	while(e->m_pFirstComponent != nullptr)
	{
		e->m_pFirstComponent->Free(handles, false);

		// Because we passed "false" above, we need to manually unlink it here.
		e->UnlinkComponent(e->m_pFirstComponent);
	}

	if (e->m_bActive) { e->Deactivate(); }

	e->DetachSelf();

	e->m_pComponents.Clear();
	e->m_pSystems.Clear();

	e->m_pFirstComponent = nullptr;

	for(uint32 i=0; i < (MAX_COMPONENT_TYPES / BITFIELD_LENGTH) + 1; i++)
		e->m_uComponentBitfield[i] = 0;

	ASSERT(g_pPool!=nullptr);
	g_pPool->Free(e);
}

uint32 ComponentEntity::NumEntities()
{
	return g_pPool == nullptr ? 0 : g_pPool->Size();
}

void ComponentEntity::SetComponentBit(uint32 uBitfieldOffset, uint32 uBitfieldIndex, bool bValue)
{
	if(bValue == true)
	{
		m_uComponentBitfield[uBitfieldOffset] |= 1 << uBitfieldIndex;
	}
	else
	{
		m_uComponentBitfield[uBitfieldOffset] &= ~(1 << uBitfieldIndex);
	}
}


void ComponentEntity::LinkComponent(ComponentType *pComp)
{
	pComp->SetNextComponent(m_pFirstComponent);

	if(m_pFirstComponent != nullptr)
	{
		m_pFirstComponent->SetPrevComponent(pComp);
	}

	m_pFirstComponent = pComp;
}

void ComponentEntity::UnlinkComponent(ComponentType *pComp)
{
	ComponentType* pNext = pComp->GetNextComponent();
	ComponentType* pPrev = pComp->GetPrevComponent();

	if(pNext != nullptr)
	{
		pNext->SetPrevComponent(pPrev);
	}

	if(pPrev != nullptr)
	{
		pPrev->SetNextComponent(pNext);
	}

	if(m_pFirstComponent == pComp)
	{
		m_pFirstComponent = pNext;
	}
}

}
