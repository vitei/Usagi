// These are the non-expanded parts of ComponentManager.
// Parts that need to be expanded to include component and system information are
// broken into separate ".bp.erb" files, named after the functionality they implement.
#include "Engine/Common/Common.h"
#include "ComponentManager.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include "Engine/Network/Network.pb.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/Signal.h"
#include "Engine/Framework/SystemKey.h"
#include "Engine/Framework/Component.pb.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Particles/ParticleComponents.pb.h"
#include "Engine/Framework/ComponentLoadHandles.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/Merge.pb.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Graphics/GPUUpdate.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Audio/AudioComponents.pb.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Framework/NewEntities.h"
#include "Engine/AI/RegisterAISystems.h"
#include "Engine/Audio/RegisterAudioSystems.h"
#include "Engine/Common/RegisterCommonSystems.h"
#include "Engine/Core/RegisterCoreSystems.h"
#include "Engine/Debug/RegisterDebugSystems.h"
#include "Engine/Framework/RegisterFrameworkSystems.h"
#include "Engine/GUI/RegisterGUISystems.h"
#include "Engine/Graphics/RegisterGraphicsSystems.h"
#include "Engine/HID/RegisterHIDSystems.h"
#include "Engine/Maths/RegisterMathsSystems.h"
#include "Engine/Memory/RegisterMemorySystems.h"
#include "Engine/Network/RegisterNetworkSystems.h"
#include "Engine/Particles/RegisterParticlesSystems.h"
#include "Engine/Physics/RegisterPhysicsSystems.h"
#include "Engine/PostFX/RegisterPostFXSystems.h"
#include "Engine/Resource/RegisterResourceSystems.h"
#include "Engine/Scene/RegisterSceneSystems.h"
#include "Engine/System/RegisterSystemSystems.h"
#include "Engine/Framework/Script/ExecuteScript.h"
#include "Engine/Framework/Script/LuaEvents.pb.h"
#include "Engine/Physics/Raycast.h"
#include "Engine/Framework/EntityLoaderHandle.h"

namespace usg
{
	void GenerateOnTriggerSignals(SystemCoordinator& systemCoordinator, Required<usg::PhysicsScene> scene);

	ComponentManager::ComponentManager(MessageDispatch* pMessageDispatch, void(*pGameSystemRegisterFunc)(SystemCoordinator&)) :
		m_systemCoordinator(),
		m_eventManager(),
		m_catchupEntities(),
		m_uChangedEntities(),
		m_internalCheckEntityTimer(),
		m_lua(),
		m_pRegisterGameSystemsFunc(pGameSystemRegisterFunc)
	{
		auto pRoot = ComponentEntity::GetRoot();
		ASSERT(pRoot == nullptr);
		ComponentEntity::InitPool(500);
		pRoot = ComponentEntity::GetRoot();
		ASSERT(pRoot != nullptr);
		m_systemCoordinator.Init(&m_eventManager, pMessageDispatch, &m_lua);
		RegisterComponents();
		RegisterSystems();
		m_systemCoordinator.LockRegistration();
	}

	ComponentManager::~ComponentManager()
	{
		auto pRoot = ComponentEntity::GetRoot();
		ASSERT(pRoot != nullptr);
		Entity freeEntity = pRoot->GetChildEntity();
		while (freeEntity)
		{
			ComponentEntity* pTmp = freeEntity->GetNextSibling();
			FreeEntityRecursive(freeEntity);
			freeEntity = pTmp;
		}

		ComponentLoadHandles handles;
		FillComponentLoadHandles(handles, ComponentEntity::GetRoot());
		m_systemCoordinator.Clear(handles);
		m_systemCoordinator.Cleanup(handles);
		GetEventManager().Clear();
		ComponentEntity::Reset();
		ComponentStats::Reset();
	}

	void ComponentManager::RegisterComponents()
	{
		m_systemCoordinator.RegisterComponent<usg::TimeComponent>();
		m_systemCoordinator.RegisterComponent<usg::EntityLoaderHandle>();
		m_systemCoordinator.RegisterComponent<usg::Identifier>();
		m_systemCoordinator.RegisterComponent<usg::SleepTag>();
		m_systemCoordinator.RegisterComponent<usg::ModelMgrComponent>();

	}

	Entity ComponentManager::GetEntityFromNetworkUID(sint64 uid)
	{
		for (GameComponents<NetworkUID>::Iterator it = GameComponents<NetworkUID>::GetIterator(); !it.IsEnd(); ++it)
		{
			if ((*it)->GetData().gameUID == uid)
			{
				return (*it)->GetEntity();
			}
		}

		return NULL;
	}

	void ComponentManager::HandleSpawnRequests()
	{
		// Handle spawn requests
		auto& ne = ComponentEntity::GetNewEntities();
		while (!ne.Empty())
		{
			const auto entityAndParent = ne.Pop();
			Entity e = entityAndParent.first;
			Entity parent = entityAndParent.second;
			if(parent)
			{
				if (e->GetParentEntity() != parent)
				{
					e->AttachToNode(parent);
				}
				if (e->HasChanged() || e->HaveChildrenChanged())
				{
					e->SetChildrenChanged();
				}
			}
			else
			{
				e->AttachToNode(ComponentEntity::GetRoot());
			}
			e->m_uSpawnFrame = m_uFrameCounter;
		}
	}

	void ComponentManager::TriggerAllSignals(float fElapsed, GFXDevice* pDevice)
	{
		HandleSpawnRequests();
		CheckEntities();

		// GetEventManager().TriggerEvents(m_systemCoordinator, rootEntity);

		while (m_catchupEntities.NumEntities() > 0)
		{
			usg::pair<Entity, double> catchupEntity = m_catchupEntities.Pop();
			float timeDiff = (float)(m_eventManager.GetTimeNow() - catchupEntity.second);
			catchupEntity.first->SetCatchupTime(timeDiff);
			catchupEntity.first->SetChanged();
		}

		Entity rootEntity = ComponentEntity::GetRoot();

		RunSignal runSignal(fElapsed);
		m_systemCoordinator.TriggerFromRoot(rootEntity, runSignal);

		for (GameComponents<usg::PhysicsScene>::Iterator it = GameComponents<usg::PhysicsScene>::GetIterator(); !it.IsEnd(); ++it)
		{
			Required<PhysicsScene> scene;
			m_componentLoadHandles.GetComponent((*it)->GetEntity(), scene);
			ExecuteRaycasts(scene, m_eventManager, m_systemCoordinator);
			GenerateOnCollisionSignals(m_systemCoordinator, scene);
			GenerateOnTriggerSignals(m_systemCoordinator, scene);
		}

		GetEventManager().TriggerEvents(m_systemCoordinator, rootEntity, m_uFrameCounter);

		LateUpdateSignal lateUpdateSignal(fElapsed);
		m_systemCoordinator.TriggerFromRoot(rootEntity, lateUpdateSignal);

		GPUHandles handles = { pDevice };
		GPUUpdateSignal gpuUpdateSignal(&handles);
		m_systemCoordinator.TriggerFromRoot(rootEntity, gpuUpdateSignal);

		for (GameComponents<usg::TimeComponent>::Iterator it = GameComponents<usg::TimeComponent>::GetIterator(); !it.IsEnd(); ++it)
		{
			++(*it)->GetData().uFrameTime;
		}

		m_uFrameCounter++;
		if (m_uFrameCounter == 0)
		{
			m_uFrameCounter = 1;
		}
	}

	Entity ComponentManager::SpawnEntityFromTemplate(const char* szFilename, Entity parent, const EntitySpawnParams& spawnParams)
	{
		ASSERT(szFilename != NULL);
		ProtocolBufferFile* pFile = ResourceMgr::Inst()->GetBufferedFile(szFilename);
		HierarchyHeader fileHeader;
		ASSERT(pFile != NULL);
		bool bReadSucceeded = pFile->Read(&fileHeader);
		ASSERT(bReadSucceeded);

		return SpawnEntityFromFile(*pFile, parent, spawnParams, true);
	}

	Entity ComponentManager::SpawnEntityFromFileWithHdr(ProtocolBufferFile& file, Entity parent, const EntitySpawnParams& spawnParams)
	{
		HierarchyHeader fileHeader;
		bool bReadSucceeded = file.Read(&fileHeader);
		ASSERT(bReadSucceeded);

		return SpawnEntityFromFile(file, parent, spawnParams, true);
	}

	void ComponentManager::ApplyTemplateToEntity(const char* szFilename, Entity root)
	{
		ASSERT(szFilename != NULL);
		ProtocolBufferFile* pFile = m_componentLoadHandles.pResourceMgr->GetBufferedFile(szFilename);
		ASSERT(pFile != NULL);
		HierarchyHeader fileHeader;
		bool bReadSucceeded = pFile->Read(&fileHeader);
		ASSERT(bReadSucceeded);

		for (uint32 i = 0; i < fileHeader.entityCount; i++)
		{
			MergeTemplateWithEntity(*pFile, root, true);
		}
	}

	void ComponentManager::PreloadAssetsFromTemplate(const char* szFilename, ComponentLoadHandles& handles)
	{
		ASSERT(szFilename != NULL);
		ProtocolBufferFile* pFile = m_componentLoadHandles.pResourceMgr->GetBufferedFile(szFilename);
		HierarchyHeader fileHeader;
		ASSERT(pFile != NULL);
		bool bReadSucceeded = pFile->Read(&fileHeader);
		ASSERT(bReadSucceeded);

		PreloadAssetsFromFile(*pFile, handles);
	}

	void ComponentManager::PreloadAssetsFromFile(usg::ProtocolBufferFile& file, ComponentLoadHandles& handles)
	{
		EntityHeader header;
		bool bReadSucceeded = file.Read(&header);
		ASSERT(bReadSucceeded);

		ComponentHeader hdr;

		while (file.Read(&hdr))
		{
			m_systemCoordinator.PreloadComponentAssets(hdr, file, handles);
		}

		for (uint32 j = 0; j < header.childEntityCount; j++)
		{
			PreloadAssetsFromFile(file, handles);
		}
	}

	void ComponentManager::SpawnHierarchyFromFile(const char* szFilename, Entity parent, const EntitySpawnParams& spawnParams)
	{
		ASSERT(szFilename != NULL);
		ProtocolBufferFile file(szFilename);
		HierarchyHeader fileHeader;
		bool bReadSucceeded = file.Read(&fileHeader);
		ASSERT(bReadSucceeded);

		for (uint32 i = 0; i < fileHeader.entityCount; i++)
		{
			SpawnEntityFromFile(file, parent, spawnParams, true);
		}
	}

	void ComponentManager::UpdateEntityIORecursive(Entity e)
	{
		Entity pChild = e->GetChildEntity();

		UpdateEntityIO(e);

		if (pChild)
		{
			ComponentEntity* pChildSibling = pChild->GetNextSibling();
			while (pChildSibling)
			{
				ComponentEntity* pTmp = pChildSibling->GetNextSibling();
				UpdateEntityIORecursive(pChildSibling);
				pChildSibling = pTmp;
			}
			UpdateEntityIORecursive(pChild);

		}
	}

	void ComponentManager::FreeEntityRecursive(Entity e)
	{
		ComponentLoadHandles handles;
		FillComponentLoadHandles(handles, e);
		Entity pChild = e->GetChildEntity();

		if (pChild)
		{
			ComponentEntity* pChildSibling = pChild->GetNextSibling();
			while (pChildSibling)
			{
				ComponentEntity* pTmp = pChildSibling->GetNextSibling();
				FreeEntityRecursive(pChildSibling);
				pChildSibling = pTmp;
			}
			FreeEntityRecursive(pChild);

		}

		RemoveEntityIO(e);
		FreeEntity(e, handles);
	}

	void ComponentManager::CheckEntity(Entity e, bool bOnlyChildren)
	{
		e->SetCatchupTime(0.0f);

		if (!bOnlyChildren)
		{
			if (e->HasPendingDeletions())
			{
				e->HandlePendingDeletes(m_componentLoadHandles);
			}
			if (e->HasChanged())
			{
				++m_uChangedEntities;
				UpdateEntityIO(e);
				e->ClearChanged();
			}
		}

		if (e->HaveChildrenChanged())
		{
			// Now update the children
			Entity child = e->GetChildEntity();
			while (child)
			{
				CheckEntity(child, false);
				child = child->GetNextSibling();
			}

			e->ClearChildrenChanged();
		}

		if (e->GetSpawnFrame() == m_uFrameCounter)
		{
			m_eventManager.TriggerEventsForEntity(m_systemCoordinator, e, ComponentEntity::GetRoot());
		}
	}

	void ComponentManager::CheckEntities()
	{
		static const uint32 uMaxBatchSize = 20;
		static Entity removedEntitites[uMaxBatchSize];
		uint32 uRemovedEntities = 0;
		// iterate through StateComponents and remove any entities and their associated components that have been flagged as killed.
		if (!GameComponents<StateComponent>::GetIterator().IsEnd())
		{
			bool bComplete = false;
			while (!bComplete)
			{
				for (GameComponents<StateComponent>::Iterator it = GameComponents<StateComponent>::GetIterator(); !it.IsEnd(); ++it)
				{
					bComplete = true;
					if ((*it)->IsActive())
					{
						if ((*it)->GetData().current == STATUS_DEAD)
						{
							Entity e = (*it)->GetEntity();

							FreeEntityRecursive(e);

							removedEntitites[uRemovedEntities] = e;
							uRemovedEntities++;

							if (uRemovedEntities >= uMaxBatchSize)
							{
								// Free this group
								m_eventManager.RegisterEntitiesRemoved(removedEntitites, uRemovedEntities);
								uRemovedEntities = 0;
							}

							bComplete = false;
							break;

						}
					}
				}
				bComplete |= GameComponents<StateComponent>::GetIterator().IsEnd();
			}
		}

		if (uRemovedEntities)
		{
			m_eventManager.RegisterEntitiesRemoved(removedEntitites, uRemovedEntities);
		}

		// iterate through entities and regenerate inputs/outputs for any that have been flagged as "changed"

		Entity e = ComponentEntity::GetRoot();

		m_uChangedEntities = 0;
		CheckEntity(e, true);
	}

	void ComponentManager::UpdateEntityIO(Entity e)
	{
		m_systemCoordinator.UpdateEntityIO(e);
	}

	void ComponentManager::RemoveEntityIO(Entity e)
	{
		m_systemCoordinator.RemoveEntityIO(e);
	}

	void ComponentManager::RegisterCatchupEntity(Entity e, double t)
	{
		m_catchupEntities.RegisterEntity(e, t);
	}

	void ComponentManager::RegisterSystems()
	{
		RegisterAISystems(m_systemCoordinator);
		RegisterAudioSystems(m_systemCoordinator);
		RegisterCommonSystems(m_systemCoordinator);
		RegisterCoreSystems(m_systemCoordinator);
		RegisterDebugSystems(m_systemCoordinator);
		RegisterFrameworkSystems(m_systemCoordinator);
		RegisterGUISystems(m_systemCoordinator);
		RegisterGraphicsSystems(m_systemCoordinator);
		RegisterHIDSystems(m_systemCoordinator);
		RegisterMathsSystems(m_systemCoordinator);
		RegisterMemorySystems(m_systemCoordinator);
		RegisterNetworkSystems(m_systemCoordinator);
		RegisterParticlesSystems(m_systemCoordinator);
		RegisterPhysicsSystems(m_systemCoordinator);
		RegisterPostFXSystems(m_systemCoordinator);
		RegisterResourceSystems(m_systemCoordinator);
		RegisterSceneSystems(m_systemCoordinator);
		RegisterSystemSystems(m_systemCoordinator);
		m_pRegisterGameSystemsFunc(m_systemCoordinator);

		m_systemCoordinator.RegisterSignal< OnEventSignal< LuaEvent > >();

		SignalRunner runner;
		OnEventSignal< LuaEvent >::FillSignalRunner<ExecuteScript>(runner, usg::GetSystemId<ExecuteScript>());
		m_systemCoordinator.RegisterSystemWithSignal(usg::GetSystemId<ExecuteScript>(), OnEventSignal< LuaEvent >::ID, runner);
	}

	void ComponentManager::LoadAndAttachComponent(const ComponentHeader& hdr, usg::ProtocolBufferFile& file, usg::Entity e)
	{
		m_systemCoordinator.LoadAndAttachComponent(hdr, file, e);
	}

	void ComponentManager::RecursivelyCallOnLoaded(Entity e, ComponentLoadHandles& handles)
	{
		m_systemCoordinator.CallOnLoaded(e, handles);

		Entity child = e->GetChildEntity();
		while (child != NULL)
		{
			RecursivelyCallOnLoaded(child, handles);
			child = child->GetNextSibling();
		}
	}

	Entity ComponentManager::SpawnEntityFromFile(ProtocolBufferFile& file, Entity parent, const EntitySpawnParams& spawnParams, bool bCallOnLoaded)
	{
		EntityHeader header;
		bool bReadSucceeded = file.Read(&header);
		ASSERT(bReadSucceeded);

		ComponentHeader hdr;
		Entity e = CreateEntity(parent);

		while (file.Read(&hdr))
		{
			LoadAndAttachComponent(hdr, file, e);
		}

		e->m_uSpawnFrame = 0;
		m_systemCoordinator.LoadEntityInitializerEvents(file, e);

		for (uint32 j = 0; j < header.childEntityCount; j++)
		{
			EntitySpawnParams params = spawnParams;
			SpawnEntityFromFile(file, e, params, false);
		}

		if (bCallOnLoaded)
		{
			if (spawnParams.HasTransform())
			{
				Required<usg::TransformComponent> trans;
				m_componentLoadHandles.GetComponent(e, trans);

				usg::Vector3f vSpawnPos = spawnParams.GetTransform().position;
				if (spawnParams.HasGlobalTransform())
				{
					Required<usg::SceneComponent, usg::FromSelfOrParents> scene;
					vSpawnPos -= scene->vOriginOffset;
				}

				if (trans.IsValid())
				{
					trans.Modify() = spawnParams.GetTransform();
					trans.Modify().position = vSpawnPos;
				}



				Required<usg::MatrixComponent> mat;
				m_componentLoadHandles.GetComponent(e, mat);
				if (mat.IsValid())
				{
					mat.Modify().matrix = spawnParams.GetTransform().rotation;
					mat.Modify().matrix.SetPos(vSpawnPos);
				}
			}

			if (spawnParams.HasOwnerNUID())
			{
				Required<usg::NetworkOwner> owner;
				m_componentLoadHandles.GetComponent(e, owner);
				if (owner.IsValid())
				{
					owner.Modify().ownerUID = spawnParams.GetOwnerNUID();
				}
			}

			if (spawnParams.HasTeam())
			{
				Required<usg::TeamComponent> team;
				m_componentLoadHandles.GetComponent(e, team);
				if (team.IsValid())
				{
					team.Modify().uTeam = spawnParams.GetTeam();
				}
			}

			ComponentLoadHandles handles;
			FillComponentLoadHandles(handles, parent);
			RecursivelyCallOnLoaded(e, handles);
		}

		return e;
	}

	void ComponentManager::MergeTemplateWithEntity(usg::ProtocolBufferFile& file, usg::Entity root, bool bCallOnLoaded)
	{
		EntityHeader header;
		bool bReadSucceeded = file.Read(&header);
		ASSERT(bReadSucceeded);

		ComponentHeader hdr;
		file.Read(&hdr);

		// The hex code below was generated thus: 0x<%= component_id('Processor.Merge').to_s(16) %>
		if (hdr.id != 0x72c7824c)
		{
			ASSERT_MSG(false, "Couldn't read Merge component!\n");
		}

		Processor::Merge merge;
		bReadSucceeded = file.Read(&merge);
		ASSERT(bReadSucceeded);

		// The optional entityWithID field specifies the entity to target.
		// If this field doesn't exist, this means we want to merge with
		// the root entity.
		Entity e = root;

		if (merge.has_entityWithID)
		{
			e = root->GetChildEntityByName(m_componentLoadHandles, merge.entityWithID);
		}

		ASSERT(e != NULL);

		while (file.Read(&hdr))
		{
			LoadAndAttachComponent(hdr, file, e);
		}

		m_systemCoordinator.LoadEntityInitializerEvents(file, e);

		EntitySpawnParams params;
		for (uint32 j = 0; j < header.childEntityCount; j++)
		{
			SpawnEntityFromFile(file, e, params, false);
		}

		if (bCallOnLoaded)
		{
			ComponentLoadHandles handles;
			FillComponentLoadHandles(handles, root);
			RecursivelyCallOnLoaded(e, handles);
		}
	}

	void ComponentManager::RegisterResourceHandles(usg::GFXDevice* pDevice, usg::ResourceMgr* pRes, usg::Scene* pScene)
	{
		m_componentLoadHandles.pDevice = pDevice;
		m_componentLoadHandles.pResourceMgr = pRes;
		m_componentLoadHandles.pScene = pScene;
	}

	void ComponentManager::FillComponentLoadHandles(ComponentLoadHandles& handlesOut, Entity parent)
	{
		handlesOut.pDevice = m_componentLoadHandles.pDevice;
		handlesOut.pScene = m_componentLoadHandles.pScene;
		handlesOut.pModelMgr = m_componentLoadHandles.pModelMgr ;
		handlesOut.pResourceMgr = m_componentLoadHandles.pResourceMgr;

		if (parent)
		{
			Required<PhysicsScene, FromSelfOrParents> physicsScene;
			m_componentLoadHandles.GetComponent(parent, physicsScene);
			if (physicsScene.IsValid())
			{
				handlesOut.pPhysicsScene = physicsScene.GetRuntimeData().pSceneData;
			}

			Required<ModelMgrComponent, FromSelfOrParents> modelMgr;
			m_componentLoadHandles.GetComponent(parent, modelMgr);
			if (modelMgr.IsValid())
			{
				handlesOut.pModelMgr = modelMgr.GetRuntimeData().pMgr;
			}
		}
	}

}