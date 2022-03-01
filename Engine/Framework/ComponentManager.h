#pragma once

#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H


#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Framework/CatchupEntities.h"
#include "Engine/Framework/MessageDispatch.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/ComponentLoadHandles.h"
#include "Engine/Framework/Script/LuaVM.h"
#include "Engine/Framework/SystemCoordinator.h"
#include "Engine/Core/Timer/ProfilingTimer.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Framework/EntitySpawnParams.h"

namespace usg
{
	class GFXDevice;
	namespace Components
	{
		typedef struct _TransformComponent TransformComponent;
	}

	class ComponentManager
	{
	public:
		ComponentManager(MessageDispatch* dispatch, void(*pGameSystemRegisterFunc)(SystemCoordinator&));
		~ComponentManager();

		void RegisterResourceHandles(usg::GFXDevice* pDevice, usg::ResourceMgr* pRes, usg::Scene* pScene);

		void TriggerAllSignals(float32 fElapsed);
		void TriggerGPUSignals(GFXDevice* pDevice);

		Entity SpawnEntityFromTemplate(const char* szFilename, Entity parent, const EntitySpawnParams& spawnParams);
		void SpawnHierarchyFromFile(const char* szFilename, Entity parent, const EntitySpawnParams& spawnParams);
		Entity SpawnEntityFromFile(ProtocolBufferFile& file, Entity parent, const EntitySpawnParams& spawnParams, bool bCallOnLoaded = true);
		Entity SpawnEntityFromFileWithHdr(ProtocolBufferFile& file, Entity parent, const EntitySpawnParams& spawnParams);

		void ApplyTemplateToEntity(const char* szFilename, Entity root);
		void MergeTemplateWithEntity(ProtocolBufferFile& file, Entity root, bool bCallOnLoaded);

		Entity GetEntityFromNetworkUID(sint64 uid);

		void UpdateEntityIORecursive(Entity e);
		void UpdateEntityIO(Entity e);

		void CheckEntities();
		void CheckEntity(Entity e, bool bOnlyChildren);

		void RemoveEntityIO(Entity e);

		void FreeEntityRecursive(Entity e);

		EventManager& GetEventManager() { return m_eventManager; }
		SystemCoordinator& GetSystemCoordinator() { return m_systemCoordinator; }
		void RegisterCatchupEntity(Entity e, double t);

		LuaVM& GetLuaVM() { return m_lua; }
		SystemCoordinator& GetCoordinator() { return m_systemCoordinator; }

		void PreloadAssetsFromTemplate(const char* szFilename, ComponentLoadHandles& handles);
		void PreloadAssetsFromFile(ProtocolBufferFile& file, ComponentLoadHandles& handles);
	private:
		void HandleSpawnRequests();
		void RegisterComponents();
		void RegisterSystems();
		void LoadAndAttachComponent(const ComponentHeader& hdr, ProtocolBufferFile& file, Entity e);
		void RecursivelyCallOnLoaded(Entity e, ComponentLoadHandles& handles);
		// TODO: These should probably be cached
		void FillComponentLoadHandles(ComponentLoadHandles& handlesOut, Entity parent);

		SystemCoordinator	m_systemCoordinator;
		EventManager		m_eventManager;
		CatchupEntities		m_catchupEntities;
		uint32				m_uChangedEntities;
		ProfilingTimer		m_internalCheckEntityTimer;
		LuaVM				m_lua;
		ComponentLoadHandles m_componentLoadHandles;
		uint32				m_uFrameCounter = 1;	// 0 is reserved

		void(*m_pRegisterGameSystemsFunc)(SystemCoordinator&);
	};

}


#endif // COMPONENT_MANAGER_H

