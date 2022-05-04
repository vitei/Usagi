/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Debug/DebugStats.h"
#include "SystemCoordinator.h"
#include "Engine/Core/stl/hash_map.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/stl/memory.h"
#include "Engine/Graphics/GPUUpdate.h"
#include "Engine/Physics/Signals/OnCollision.h"
#include "Engine/Physics/Signals/OnTrigger.h"
#include "Engine/Physics/Signals/OnRaycastHit.h"
#include "Engine/Core/stl/algorithm.h"
#include "Engine/Framework/EventManager.h"

using namespace usg;

namespace usg
{
	struct ProtocolBufferReaderData
	{
		void(*pPtrToReader)(ProtocolBufferFile& file, void* data);
		memsize uDataSize;
	};

	struct SystemCoordinator::InternalData
	{
		usg::vector<SystemHelper> systemHelpers;
		usg::vector<ComponentHelper> componentHelpers;
		usg::hash_map<uint32, uint32> componentHashLookUp; // component-hash => id mapping
		usg::hash_map<uint32, usg::vector<SignalRunner>> signalRunners;
		usg::hash_map<uint32, ProtocolBufferReaderData> protocolBufferReaders;

		void Clear()
		{
			systemHelpers.clear();
			componentHelpers.clear();
			componentHashLookUp.clear();
			signalRunners.clear();
		}
	};

	void SystemCoordinator::RegisterProtocolBufferTypeInt(const uint32 uTypeId, memsize uDataSize, void(*pPtrToReader)(ProtocolBufferFile& file, void* data))
	{
		auto& readerData = m_pInternalData->protocolBufferReaders[uTypeId];
		readerData.pPtrToReader = pPtrToReader;
		readerData.uDataSize = uDataSize;
	}

	void SystemCoordinator::LoadEntityInitializerEvents(ProtocolBufferFile& file, Entity e)
	{
		usg::vector<uint8> buf;
		buf.resize(64);

		InitializerEventHeader eventHeader;
		while (file.Read(&eventHeader))
		{
			ASSERT(m_pInternalData->protocolBufferReaders.count(eventHeader.id) > 0);
			auto& reader = m_pInternalData->protocolBufferReaders[eventHeader.id];
			if (reader.uDataSize > buf.size())
			{
				buf.resize(reader.uDataSize);
			}
			reader.pPtrToReader(file, &buf[0]);
			m_pEventManager->RegisterEventWithEntityAtTime(e, eventHeader.id, &buf[0], reader.uDataSize, ON_ENTITY, 0);
		}
	}

	void SystemCoordinator::RegisterSignalInt(const uint32 uSignalId, void(*pPtrToInitializer)(SystemCoordinator& sc))
	{
		auto& runners = m_pInternalData->signalRunners;
		auto data = runners.find(uSignalId);
		if (data == runners.end())
		{
			runners[uSignalId].reserve(16);
			pPtrToInitializer(*this);
		}
	}

	void SystemCoordinator::RegisterSystemWithSignal(uint32 uSystemId, uint32 uSignalId, const SignalRunner& runner)
	{
		ASSERT(m_pInternalData->signalRunners.count(uSignalId) > 0);
		auto& runners = m_pInternalData->signalRunners[uSignalId];
		runners.push_back(runner);
	}

	void SystemCoordinator::Trigger(Entity e, Signal& sig, uint32 targets)
	{
		auto& runners = m_pInternalData->signalRunners[sig.uId];
		{
			for (auto& runner : runners)
			{
#ifdef ENABLE_SYSTEM_PROFILE_TIMERS
				const uint32 systemID = runner.systemID;
				ASSERT(systemID != SignalRunner::INVALID_SYSTEM_ID);
				SystemHelper& helper = m_systemHelpers[systemID];

				helper.timer.Start();
#endif

				runner.Trigger(e, &sig, runner.systemID, targets, runner.userData);

#ifdef ENABLE_SYSTEM_PROFILE_TIMERS
				helper.timer.Stop();
#endif
			}
		}
	}

	void SystemCoordinator::TriggerFromRoot(Entity e, Signal& sig)
	{
		auto& runners = m_pInternalData->signalRunners[sig.uId];
		{
			for (auto& runner : runners)
			{
#ifdef ENABLE_SYSTEM_PROFILE_TIMERS
				const uint32 systemID = runner.systemID;
				ASSERT(systemID != SignalRunner::INVALID_SYSTEM_ID);
				SystemHelper& helper = m_systemHelpers[systemID];

				helper.timer.Start();
#endif

				runner.TriggerFromRoot(e, &sig, runner.systemID, runner.userData);

#ifdef ENABLE_SYSTEM_PROFILE_TIMERS
				helper.timer.Stop();
#endif
			}
		}
	}

	SystemCoordinator::SystemCoordinator()
		: m_pMemPoolBuffer(nullptr)
		, m_pEventManager(nullptr)
		, m_pMessageDispatch(nullptr)
		, m_pLuaVM(nullptr)
		, m_uBitfieldsPerKey(0)
		, m_pSystemKeyBuffer(nullptr)
		, m_pSystemData(nullptr)
	{
		static const size_t POOL_SIZE = 256 * 1024;
		m_pMemPoolBuffer = mem::Alloc(MEMTYPE_STANDARD, ALLOC_SYSTEM, POOL_SIZE, 4U);
		m_memPool.Initialize(m_pMemPoolBuffer, POOL_SIZE);

		m_pInternalData = vnew(ALLOC_OBJECT)InternalData();

		RegisterSignal<RunSignal>();
		const uint32 id = LateUpdateSignal::ID;
		ASSERT(id >1);
		RegisterSignal<LateUpdateSignal>();
		RegisterSignal<GPUUpdateSignal>();
		RegisterSignal<OnRaycastHitSignal>();
		RegisterSignal<OnCollisionSignal>();
		RegisterSignal<OnTriggerSignal>();
	}
}

SystemCoordinator::~SystemCoordinator()
{
	ASSERT(!m_pInternalData);
}

void SystemCoordinator::Cleanup(ComponentLoadHandles& handles)
{
	Clear(handles);

	vdelete m_pInternalData;
	m_pInternalData = nullptr;

	m_memPool.FreeGroup(0);
	mem::Free(MEMTYPE_STANDARD, m_pMemPoolBuffer);
	m_pMemPoolBuffer = nullptr;
}

void SystemCoordinator::Init(EventManager* pEventManager, MessageDispatch* pMessageDispatch, LuaVM* pLuaVM)
{
	ASSERT(m_pEventManager == nullptr && m_pMessageDispatch == nullptr && m_pLuaVM == nullptr);
	m_pEventManager = pEventManager;
	m_pMessageDispatch = pMessageDispatch;
	m_pLuaVM = pLuaVM;
}

void SystemCoordinator::RegisterComponent(uint32 uComponentId, uint32 uComponentHash, const ComponentHelper& h)
{
	ASSERT(uComponentId < ComponentEntity::MAX_COMPONENT_TYPES);
	if (GameComponentMgr::IsInitialized(uComponentId))
	{
		return;
	}

	if (uComponentId >= m_pInternalData->componentHelpers.size())
	{
		m_pInternalData->componentHelpers.reserve(uComponentId + 64);
		while (m_pInternalData->componentHelpers.size() <= uComponentId)
		{
			m_pInternalData->componentHelpers.push_back_uninitialized();
		}
	}
	ComponentHelper& helper = m_pInternalData->componentHelpers[uComponentId];
	helper = h;
	helper.Init();
	if (uComponentHash != INVALID_PB_ID)
	{
		m_pInternalData->componentHashLookUp[uComponentHash] = uComponentId;
	}
}

void SystemCoordinator::UpdateSystemList()
{
	uint32* pActiveSystems = ComponentStats::GetComponentFlags();
	SystemData* pData = m_pSystemData;
	KeyIndex* pKey = m_pSystemKeyBuffer;
	const size_t uSystemCount = m_pInternalData->systemHelpers.size();
	for (sint32 i = 0; i < (sint32)uSystemCount; i++)
	{
		if (!pData->bSystemActive)
		{
			bool bCanBeRun = true;
			for (uint32 j = 0; j < pData->uKeyCount; j++)
			{
				bCanBeRun &= (pKey[j].uCmpValue & pActiveSystems[pKey[j].uIndex]) == pKey[j].uCmpValue;
			}
			pData->bSystemActive = bCanBeRun;
		}
		pKey += pData->uKeyCount;
		pData++;
	}

}


void SystemCoordinator::UpdateEntityIO(ComponentEntity* e)
{
	if (ComponentStats::GetFlagsDirty())
	{
		// Update our list of potentially active systems
		UpdateSystemList();
		ComponentStats::ClearFlagsDirty();
	}
	uint32 uCurrentlyRunningSystems[16];
	usg::MemSet(&uCurrentlyRunningSystems, 0, sizeof(uCurrentlyRunningSystems));
	StringPointerHash<GenericInputOutputs*>& systems = e->GetSystems();
	for (StringPointerHash<GenericInputOutputs*>::Iterator it = systems.Begin(); !it.IsEnd(); ++it)
	{
		const uint32 uKey = it.GetKey().Get();
		const uint32 uIndex = uKey - 1;
		uCurrentlyRunningSystems[uIndex / 32] |= (1<<(uIndex%32));
	}

	e->SetOnCollisionMask(0);

	SystemData* pData = m_pSystemData;
	KeyIndex* pKey = m_pSystemKeyBuffer;
	uint32 uSystemCount = (uint32)m_pInternalData->systemHelpers.size();

	bool bEntityHasRequiredComponents = true;
	for (uint32 i=0; i<uSystemCount; i++)
	{
		if(pData->bSystemActive)
		{
			const SystemHelper& helper = m_pInternalData->systemHelpers[i];
			uint32* pEntityCmp = e->GetRawComponentBitfield();

			bEntityHasRequiredComponents = true;
			for (uint32 j = 0; j < pData->uKeyCount; j++)
			{
				bEntityHasRequiredComponents &= (pKey[j].uCmpValue & pEntityCmp[pKey[j].uIndex]) == pKey[j].uCmpValue;
			}

			const uint32 uIndex = helper.systemTypeID;
			ASSERT(uIndex / 32 < ARRAY_SIZE(uCurrentlyRunningSystems));
			const bool bEntityIsCurrentlyRunning = (uCurrentlyRunningSystems[uIndex / 32] & (1 << (uIndex % 32))) != 0;

			if (bEntityIsCurrentlyRunning || bEntityHasRequiredComponents)
			{
				ComponentGetter componentGetter(e);
				const bool bIsRunning = helper.UpdateInputOutputs(componentGetter, bEntityHasRequiredComponents, bEntityIsCurrentlyRunning);
				if (helper.bIsCollisionListener && bIsRunning)
				{
					e->SetOnCollisionMask(e->GetOnCollisionMask() | helper.uOnCollisionMask);
				}
			}
		}

		pKey += pData->uKeyCount;
		pData++;
	}
}

void SystemCoordinator::RemoveEntityIO(ComponentEntity* e)
{
	StringPointerHash<GenericInputOutputs*>& systems = e->GetSystems();
	while (systems.Count())
	{
		StringPointerHash<GenericInputOutputs*>::Iterator it = systems.Begin();
		const uint32 uKey = it.GetKey().Get();
		const uint32 uIndex = uKey - 1;
		const SystemHelper& helper = m_pInternalData->systemHelpers[uIndex];
		if (helper.RemoveInputOutputs != nullptr)
		{
			helper.RemoveInputOutputs(e);
		}
	}
}

SystemCoordinator::SystemHelper& SystemCoordinator::GetSystemHelper(uint32 uSystemId)
{
	auto& helpers = m_pInternalData->systemHelpers;
	if (uSystemId >= helpers.size())
	{
		helpers.reserve(uSystemId + 64);
		while (helpers.size() <= uSystemId)
		{
			helpers.push_back_uninitialized();
		}
	}
	return helpers[uSystemId];
}

void SystemCoordinator::CleanupSystems()
{
	for(uint32 i = 0; i < m_pInternalData->systemHelpers.size(); ++i)
	{
		if (m_pInternalData->systemHelpers[i].Cleanup != nullptr)
		{
			m_pInternalData->systemHelpers[i].Cleanup();
		}
	}
}

void SystemCoordinator::CleanupComponents(ComponentLoadHandles& handles)
{
	GameComponentMgr::Cleanup(handles);
}

void SystemCoordinator::PreloadComponentAssets(const usg::ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles, usg::set<usg::string>& referencedEntities)
{
	if (m_pInternalData->componentHashLookUp.count(hdr.id) == 0)
	{
		file.AdvanceBytes(hdr.byteLength);
		return;
	}
	const uint32 uComponentID = m_pInternalData->componentHashLookUp[hdr.id];
	if(m_pInternalData->componentHelpers[uComponentID].PreloadComponentAssets != nullptr)
	{
		m_pInternalData->componentHelpers[uComponentID].PreloadComponentAssets(hdr, file, handles, referencedEntities);
	}
}

void SystemCoordinator::LoadAndAttachComponent(const ComponentHeader& hdr, ProtocolBufferFile& file, Entity e)
{
	if (m_pInternalData->componentHashLookUp.count(hdr.id) == 0)
	{
		file.AdvanceBytes(hdr.byteLength);
		return;
	}
	const uint32 uComponentID = m_pInternalData->componentHashLookUp[hdr.id];
	if (m_pInternalData->componentHelpers[uComponentID].LoadAndAttachComponent != nullptr)
	{
		m_pInternalData->componentHelpers[uComponentID].LoadAndAttachComponent(file, e);
	}
}

void SystemCoordinator::CallOnLoaded(ComponentEntity* e, ComponentLoadHandles& handles)
{
	for(uint32 i = 0; i < m_pInternalData->componentHelpers.size(); ++i)
	{
		if (e->GetComponentBitfield(i / BITFIELD_LENGTH) & (1<<(i%BITFIELD_LENGTH)))
		{
			if (m_pInternalData->componentHelpers[i].CallOnLoaded != nullptr)
			{
				m_pInternalData->componentHelpers[i].CallOnLoaded(e, handles);
			}
			else
			{
				e->GetComponent(i)->SetLoaded();
			}
		}
	}
}

void SystemCoordinator::LockRegistration()
{
	// Calculate System keys
	m_uBitfieldsPerKey = ((uint32)m_pInternalData->systemHelpers.size() / BITFIELD_LENGTH) + 1;
	m_pSystemData = (SystemData*)m_memPool.Allocate(m_pInternalData->systemHelpers.size() * sizeof(SystemData), 4, 0, ALLOC_SYSTEM);

	// Sort Signal Runners by priority
	for (auto& runners : m_pInternalData->signalRunners)
	{
		usg::sort(runners.second.begin(), runners.second.end(), [](const SignalRunner& a, const SignalRunner& b) {
			return a.priority < b.priority;
		});
	}

	uint32 uKeyCount = 0;

	// First figure out how many keys
	for(uint32 i = 0; i < m_pInternalData->systemHelpers.size(); ++i)
	{
		if (m_pInternalData->systemHelpers[i].GetSystemKey != nullptr)
		{
			for (uint32 j = 0; j < m_uBitfieldsPerKey; ++j)
			{
				// Only check against relevant keys
				if (m_pInternalData->systemHelpers[i].GetSystemKey(j) != 0)
				{
					uKeyCount++;
				}
			}
		}
	}

	m_pSystemKeyBuffer = (KeyIndex*)m_memPool.Allocate(uKeyCount * sizeof(KeyIndex), 4, 0, ALLOC_SYSTEM);

	uKeyCount = 0;

	for (uint32 i = 0; i < m_pInternalData->systemHelpers.size(); ++i)
	{
		SystemData* pSystem = &m_pSystemData[i];
		KeyIndex* pKeyBufferOffset = &m_pSystemKeyBuffer[uKeyCount];
		uint32 uSystemKeyCount = 0;
		if (m_pInternalData->systemHelpers[i].GetSystemKey != nullptr)
		{
			for (uint32 j = 0; j < m_uBitfieldsPerKey; ++j)
			{
				// Only check against relevant keys
				if (m_pInternalData->systemHelpers[i].GetSystemKey(j) != 0)
				{
					pKeyBufferOffset->uCmpValue = m_pInternalData->systemHelpers[i].GetSystemKey(j);
					pKeyBufferOffset->uIndex = j;
					pKeyBufferOffset++;
					uSystemKeyCount++;
					uKeyCount++;
				}
			}
			pSystem->uKeyCount = uSystemKeyCount;
			pSystem->bSystemActive = false;
			pSystem->pKeys = pKeyBufferOffset;
		}
	}
}

#ifdef ENABLE_SYSTEM_PROFILE_TIMERS
void SystemCoordinator::RegisterTimers()
{
	Color colorSystemTimer( 0.4f, 1.0f, 0.4f );

	for(uint32 i = 0; i < m_systemHelpers.GetSize(); ++i)
	{
		SystemHelper& helper = m_systemHelpers[i];
		DebugStats::Inst()->RegisterTimer(helper.systemName, &helper.timer, colorSystemTimer, 0.04f);
	}
}

void SystemCoordinator::ClearTimers()
{
	for(uint32 i = 0; i < m_systemHelpers.GetSize(); ++i)
	{
		m_systemHelpers[i].timer.Clear();
	}
}
#endif

void SystemCoordinator::Clear(ComponentLoadHandles& handles)
{
	CleanupComponents(handles);
	CleanupSystems();

	m_pInternalData->Clear();

	m_pEventManager = nullptr;
	m_pMessageDispatch = nullptr;

	m_uBitfieldsPerKey = 0;
	if(m_pSystemKeyBuffer != nullptr)
	{
		m_memPool.Deallocate(m_pSystemKeyBuffer);
		m_pSystemKeyBuffer = nullptr;
	}

	// HACK: Remove when StringPointerHash iterator is fixed
	m_memPool.FreeGroup(0);
}