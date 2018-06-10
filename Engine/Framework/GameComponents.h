/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USAGI_GAMECOMPONENTS_H_
#define _USAGI_GAMECOMPONENTS_H_
#include "Engine/Common/Common.h"
#include "Engine/Framework/SystemKey.h"
#include "Engine/Framework/Component.pb.h"
#include "Engine/Core/String/StringCRC.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Memory/UnTypesafeFastPool.h"
#include "ComponentStats.h"
#include "ComponentEntity.h"
#include "Component.h"
#include "System.h"
#include "ComponentSystemInputOutputs.h"
#include "Engine/Framework/ComponentProperties.h"

namespace usg
{

template<typename T>
struct ComponentInitializer
{
	static void Init(T* pComponent)
	{
		// By default, use the initializer provided by Protocol Buffers
		ProtocolBufferFields<T>::Init(pComponent);
	}
};

// This is a shared base class for the GameComponents<T> template classes. Keep as much code as possible here to reduce code bloat.
class GameComponentMgr
{
protected:
	static void InitPool(uint32 uTypeId, memsize uElementSize, uint32 uPoolChunkSize, void(*pComponentConstructorCaller)(void*), void(*pComponentDestructorCaller)(void*), void(*pActivate)(ComponentType*, Entity), void(*pDeactivate)(ComponentType*, ComponentLoadHandles& handles, bool));
	static void FreePool(uint32 uTypeId);
	static ComponentType* AllocFromPool(uint32 uTypeId);
	static UnTypesafeFastPool* GetPool(uint32 uTypeId);
public:
	static void Cleanup(ComponentLoadHandles& handles);
	static bool IsInitialized(uint32 uTypeId);
	static ComponentType* Create(uint32 uTypeId, Entity e);
	static void Free(ComponentType* pComponent, ComponentLoadHandles& handles, bool bRemoveFromEntity);
};

namespace gamecomponents_details
{
	template<typename T>
	inline static void ConstructorCaller(void* pRaw)
	{
		new (pRaw) Component<T>();
	}

	template<typename T>
	inline static void DestructorCaller(void* pRaw)
	{
		Component<T>* pComponent = (Component<T>*)pRaw;
		pComponent->~Component<T>();
	}

	template<typename T>
	inline static void CallActivate(ComponentType* pComponent, Entity e)
	{
		Component<T>* pComponentSpecialized = (Component<T>*)pComponent;
		ComponentInitializer<T>::Init(&pComponentSpecialized->GetData());
		pComponentSpecialized->Activate(e);
	}

	template<typename T>
	inline static void CallDeactivate(ComponentType* pComponent, ComponentLoadHandles& handles, bool bRemoveFromEntity)
	{
		Component<T>* pComponentSpecialized = (Component<T>*)pComponent;
		pComponentSpecialized->Deactivate(handles, bRemoveFromEntity);
	}
}

template<typename T>
class GameComponents : public GameComponentMgr
{
public:
	typedef typename UnTypesafeFastPool::DynamicIterator<Component<T>> Iterator;
	static constexpr uint32 uPoolChunkSize = ComponentProperties<T>::FastPoolChunkSize > 0 ? ComponentProperties<T>::FastPoolChunkSize : 10;

public:
	static void Init()
	{
		InitPool(GetTypeID(), sizeof(Component<T>) ,uPoolChunkSize, gamecomponents_details::ConstructorCaller<T>, gamecomponents_details::DestructorCaller<T>, gamecomponents_details::CallActivate<T>, gamecomponents_details::CallDeactivate<T>);
	}
	
	static uint32 GetTypeID()
	{
		return Component<T>::GetTypeID();
	}

	static T* Create(Entity e)
	{
		return &((Component<T>*)GameComponentMgr::Create(GetTypeID(), e))->GetData();
	}

	static void Free(Component<T>* c, ComponentLoadHandles& handles, bool bRemoveFromEntity = true)
	{
		GameComponentMgr::Free(c, handles, bRemoveFromEntity);
	}

	static void RequestFree(Entity uEntity)
	{
		GetComponent(uEntity)->RequestFree();
	}

	static Component<T>* GetComponent(Entity uEntity)
	{
		if (uEntity == NULL)
		{
			return NULL;
		}

		return (Component<T>*) uEntity->GetComponent(GetTypeID());
	}

	static T* GetComponentData(Entity uEntity)
	{
		Component<T>* c = GetComponent(uEntity);
		return c ? &c->GetData() : NULL;
	}
	
	static Iterator GetIterator()
	{
		auto pPool = GetPool(GetTypeID());
		ASSERT(pPool != nullptr);
		return pPool->_BeginDynamic<Component<T>>();
	}
};

struct ComponentGetterBase
{
	static void* GetComponentBase(Entity e, uint32 uComponentId, sint32 iSearchMask);
};

template<typename T, typename SearchMask>
struct ComponentGetterInt : public ComponentGetterBase
{
	static Component<T>* GetComponent(Entity uEntity)
	{
		return (Component<T>*)GetComponentBase(uEntity, Component<T>::GetTypeID(), SearchMask::value);
	}
};

template<typename T, typename ComponentType>
struct ComponentGetterInt< T, FromParentWith<ComponentType> >
{
	static Component<T>* GetComponent(Entity uEntity)
	{
		Component<ComponentType>* component = ComponentGetterInt<ComponentType, FromParents>::GetComponent(uEntity);
		if(component == NULL) { return NULL; }
		return ComponentGetterInt<T, FromSelf>::GetComponent(component->GetEntity());
	}
};

struct UnsafeComponentGetter
{
public:
	template<typename T, typename SearchMask>
	bool GetComponent(Entity uEntity, Optional<T, SearchMask>& optional) const
	{
		return GetComponentImpl(uEntity, optional);
	}

	template<typename T, typename SearchMask>
	bool GetComponent(Entity uEntity, Required<T, SearchMask>& required) const
	{
		return GetComponentImpl(uEntity, required);
	}

protected:
	// Don't want this being constructed just anywhere
	UnsafeComponentGetter() {};

	template<typename T, typename SearchMask>
	static bool GetComponentImpl(Entity uEntity, Optional<T, SearchMask>& optional) 
	{
		optional = Optional<T, SearchMask>(ComponentGetterInt<T, SearchMask>::GetComponent(uEntity));
		return true;
	}

	template<typename T, typename SearchMask>
	static bool GetComponentImpl(Entity uEntity, Required<T, SearchMask>& required) 
	{
		Component<T>* c = ComponentGetterInt<T, SearchMask>::GetComponent(uEntity);

		bool exists = c != NULL;
		if (exists) { required = Required<T, SearchMask>(*c); }

		return exists;
	}
};

struct ComponentLoadHandles : public ComponentLoadHandlesBase, public UnsafeComponentGetter
{
	friend ComponentManager;
private:
	ComponentLoadHandles() {};
};





bool GetComponent(Entity e, uint32 uComponentType, sint32 iSearchMask, ComponentWrapperBase& wrapper);



template<typename T>
bool HasComponent(Entity uEntity)
{
	return uEntity->HasComponent(Component<T>::GetTypeID());
}
	
template<typename T>
inline void OnActivate(Component<T>&)
{
}

// This function may be called more than once if templates are merged
// into an entity tree. The second parameter is true if OnLoaded was
// previously called for this component, and can be used to skip
// duplicate initializations.
template<typename T>
inline void OnLoaded(Component<T>&, ComponentLoadHandles& handles, bool bWasPreviouslyCalled);

// This function allows us to preload resources mentioned by a component
// without attaching it to any entity.
class GFXDevice;
template<typename T>
inline void PreloadComponentAssets(const ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles)
{
	file.AdvanceBytes(hdr.byteLength);
}


// Component helpers
// These have to go in here because they depend on GameComponents for
// their functionality... nasty
template<typename T>
void Component<T>::Activate(Entity e)
{
	ComponentType::Activate(e);
	OnActivate(*this);
}

template<typename T>
bool Component<T>::IsActive() const
{
	return m_uEntity != NULL && m_uEntity->HasComponent(GetTypeID());
}



template<typename T>
inline void OnDeactivate(Component<T>&, ComponentLoadHandles& handles)
{
}

template<typename T>
void Component<T>::Deactivate(ComponentLoadHandles& handles, bool bRemoveFromEntity)
{
	OnDeactivate(*this, handles);
	if(bRemoveFromEntity)
	{
		m_uEntity->SetComponent(GetTypeID(), NULL);
	}
	ComponentStats::DeactivatedComponent();
}

template<typename T>
void Component<T>::Free(ComponentLoadHandles& handles, bool bRemoveFromEntity)
{
	GameComponents<T>::Free(this, handles, bRemoveFromEntity);
}

// Helper macros
#define GET_ENTITY_COMPONENT( entity, type) \
	&GameComponents< type >::GetComponent( (entity) )->GetData()

#define COMPONENT_FROM_ENTITY( type, entity, varName ) \
	type * varName = &GameComponents< type >::GetComponent( entity )->GetData();

namespace Components {}
namespace Systems {}
using namespace Components;
using namespace Systems;
namespace Events {}
using namespace Events;

namespace ai
{
namespace Components {}
namespace Systems {}
namespace Events {}
using namespace Components;
using namespace Systems;
using namespace Events;
}

}

namespace Components {}
namespace Systems {}
using namespace Components;
using namespace Systems;
namespace Events {}
using namespace Events;

//Probably want to move this into the Systems namespace later...
using usg::Required;
using usg::Optional;

#endif
