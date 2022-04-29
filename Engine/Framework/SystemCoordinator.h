/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/

#ifndef _SYSTEM_COORDINATOR_H
#define _SYSTEM_COORDINATOR_H

#include "Engine/Framework/Signal.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/Component.pb.h"
#include "Engine/Framework/SystemId.h"
#include "Engine/Framework/ComponentHelper.h"
#include "Engine/Core/Timer/ProfilingTimer.h"

// Uncomment the following to enable system-specific profile timers.
// These are useful when wanting to profile systems individually, but
// they are quite slow so generally we leave them turned off.
// Don't forget you need to have both this and ENABLE_PROFILE_TIMERS
// defined in order to get system timers working!
//#define ENABLE_SYSTEM_PROFILE_TIMERS

namespace usg {

class EventManager;
class MessageDispatch;
class LuaVM;
class ComponentEntity;
class SystemCoordinator
{
public:
	SystemCoordinator();
	~SystemCoordinator();

	// Do this before registering systems.
	void Init(EventManager* pEventManager, MessageDispatch* pMessageDispatch, LuaVM* pLuaVM);
	void Cleanup(ComponentLoadHandles& handles);

	template<typename SIGNAL> void RegisterSignal();
	template<typename SYSTEM> void RegisterSystem();

	void RegisterSystemWithSignal(uint32 uSystemId, uint32 uSignalId, const SignalRunner& runner);

	void UpdateEntityIO(ComponentEntity* e);
	void RemoveEntityIO(ComponentEntity* e);

	void RegisterComponent(uint32 uComponentId, uint32 uComponentHash, const ComponentHelper& h);
	template<typename COMPONENT> void RegisterComponent();
	void PreloadComponentAssets(const usg::ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles, usg::set<usg::string>& referencedEntities);
	void LoadAndAttachComponent(const ComponentHeader& hdr, ProtocolBufferFile& file, Entity pEntity);
	void LoadEntityInitializerEvents(ProtocolBufferFile& file, Entity e);
	void CallOnLoaded(ComponentEntity* e, ComponentLoadHandles& handles);

	// Do this once all systems are registered; you can't register any more
	// Systems after this.
	void LockRegistration();

	void Trigger(Entity e, Signal& sig, uint32 targets);
	void TriggerFromRoot(Entity e, Signal& sig);

#ifdef ENABLE_SYSTEM_PROFILE_TIMERS
	void RegisterTimers();
	void ClearTimers();
#endif

	void Clear(ComponentLoadHandles& handles);

	template<typename T>
	void RegisterProtocolBufferType()
	{
		RegisterProtocolBufferTypeInt(ProtocolBufferFields<T>::ID, sizeof(T), ReadProtocolBufferData<T>);
	}

protected:
private:
	template<typename T>
	static void ReadProtocolBufferData(ProtocolBufferFile& file, void* pOut)
	{
		T* pDst = (T*)pOut;
		file.Read(pDst);
	}

	void UpdateSystemList();

	struct InternalData;
	InternalData* m_pInternalData = nullptr;

	// Pool for the generic iterator lists
	void *m_pMemPoolBuffer;
	MemHeap m_memPool;

	void RegisterProtocolBufferTypeInt(const uint32 uTypeId, memsize uDataSize, void(*pPtrToReader)(ProtocolBufferFile& file, void* data));
	void RegisterSignalInt(const uint32 uSignalId, void(*pPtrToInitializer)(SystemCoordinator& sc));

	// Signal helper.  This allows us to perform initialisation the first time
	// a System registers itself with a new Signal (i.e. when its handlers list is
	// initialised).  We default to no initialisation, but can specialise the
	// function if needed.
	template<typename SIGNAL> struct SignalHelper { static void RegisterSignal(SystemCoordinator&) {} };
	template<typename SIGNAL> friend struct SignalHelper;

	// Event/message dispatch.  Required for OnEvent signal initialisation.
	EventManager*    m_pEventManager;
	MessageDispatch* m_pMessageDispatch;
	LuaVM*           m_pLuaVM;

	struct KeyIndex
	{
		uint32 uIndex;
		uint32 uCmpValue;
	};

	struct SystemData
	{
		uint32		uKeyCount;
		KeyIndex*	pKeys;
		bool		bSystemActive;
	};


	struct SystemHelper
	{
		const char* systemName;
		uint32 systemTypeID = 0xffffffff;

		bool bIsCollisionListener;
		uint32 uOnCollisionMask;

		void (*Cleanup)() = nullptr;
		uint32 (*GetSystemKey)(uint32 uOffset) = nullptr;
		bool (*UpdateInputOutputs)(ComponentGetter& getter, bool bEntityHasRequiredComponents, bool bEntityIsCurrentlyRunning) = nullptr;
		void (*RemoveInputOutputs)(ComponentEntity* e) = nullptr;

#ifdef ENABLE_SYSTEM_PROFILE_TIMERS
		usg::ProfilingTimer timer;
#endif
	};

	void CleanupSystems();
	SystemHelper& GetSystemHelper(uint32 uSystemId);

	// System keys
	uint32 m_uBitfieldsPerKey;

	KeyIndex* m_pSystemKeyBuffer;
	SystemData* m_pSystemData;

	template<typename COMPONENT>
	static void DoLoadAndAttachComponent(ProtocolBufferFile& file, ComponentEntity* pEntity);

	void CleanupComponents(ComponentLoadHandles& handles);

	template<typename COMPONENT>
	ComponentHelper GenerateComponentHelper();
};

template<typename SIGNAL>
void SystemCoordinator::RegisterSignal()
{
	RegisterSignalInt(SIGNAL::ID, SignalHelper<SIGNAL>::RegisterSignal);
}

template<typename SYSTEM>
void SystemCoordinator::RegisterSystem()
{
	static const uint32 systemID = GetSystemId<SYSTEM>();
	SystemHelper& helper = GetSystemHelper(systemID);
	helper.systemName = SYSTEM::Name();
	helper.systemTypeID = systemID;
	helper.Cleanup = CleanupSystem<SYSTEM>;
	helper.bIsCollisionListener = SYSTEM::AUTOGENERATE_GET_COLLIDER_INPUTS == ON;
	helper.uOnCollisionMask = SYSTEM::OnCollisionMask;
	helper.GetSystemKey = GetSystemKey<SYSTEM>;
	helper.UpdateInputOutputs = UpdateInputOutputs<SYSTEM>;
	helper.RemoveInputOutputs = ComponentSystemInputOutputs<SYSTEM>::RemoveInputOutputs;
}

template<typename COMPONENT>
void SystemCoordinator::DoLoadAndAttachComponent(ProtocolBufferFile& file, ComponentEntity *pEntity)
{
	COMPONENT* pComponent = &((Component<COMPONENT>*)GameComponentMgr::Create(Component<COMPONENT>::GetTypeID(),pEntity))->GetData();
	bool readSuccess = file.Read(pComponent);
	ASSERT(readSuccess);
}

namespace systemcoordinator_details
{
	template<typename C, bool HasOnLoaded>
	struct OnLoadedSetter;

	template<typename C>
	struct OnLoadedSetter<C, true>
	{
		static void CallOnLoaded(ComponentEntity* e, ComponentLoadHandles& handles)
		{
			Component<C>* component = GameComponents<C>::GetComponent(e);

			if (component != NULL)
			{
				OnLoaded(*component, handles, component->WasLoaded());
				component->SetLoaded();
			}
		}

		static void Set(ComponentHelper& h)
		{
			h.CallOnLoaded = CallOnLoaded;
		}
	};

	template<typename C>
	struct OnLoadedSetter<C, false>
	{
		static void Set(ComponentHelper& h)
		{
			h.CallOnLoaded = nullptr;
		}
	};
}

template<typename COMPONENT>
ComponentHelper SystemCoordinator::GenerateComponentHelper()
{
	ComponentHelper helper;
	helper.Init = GameComponents<COMPONENT>::Init;
	systemcoordinator_details::OnLoadedSetter<COMPONENT, ComponentProperties<COMPONENT>::HasOnLoaded>::Set(helper);

	static constexpr uint32 uComponentHash = ProtocolBufferFields<COMPONENT>::ID;
	if (uComponentHash != INVALID_PB_ID)
	{
		helper.PreloadComponentAssets = usg::PreloadComponentAssets<COMPONENT>;
		helper.LoadAndAttachComponent = DoLoadAndAttachComponent<COMPONENT>;
	}
	else
	{
		helper.PreloadComponentAssets = NULL;
		helper.LoadAndAttachComponent = NULL;
	}
	return helper;
}

template<typename COMPONENT>
void SystemCoordinator::RegisterComponent()
{
	RegisterComponent(Component<COMPONENT>::GetTypeID(), ProtocolBufferFields<COMPONENT>::ID, GenerateComponentHelper<COMPONENT>());
}

template<typename ComponentType>
void RegisterComponent(SystemCoordinator& systemCoordinator)
{
	ASSERT(false);
}

template<typename EventType>
void RegisterEvent(SystemCoordinator& systemCoordinator);

}

#endif //_SYSTEM_COORDINATOR_H
