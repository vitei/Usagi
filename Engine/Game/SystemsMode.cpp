#include "Engine/Common/Common.h"
#include "Engine/Framework/ComponentEntity.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Framework/Messenger.h"
#include "Engine/Network/UsagiNetwork.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Framework/ComponentLoadHandles.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Framework/EntityLoader.h"
#include "Engine/Framework/EntityLoaderHandle.h"
#include "Engine/Framework/ComponentManager.h"
#include "SystemsMode.h"


namespace usg
{
	struct SystemsMode::InternalData
	{
		GFXDevice* pDevice = nullptr;
		NetManager* pNetManager = nullptr;
		unique_ptr<ComponentManager> pComponentManager;
		unique_ptr<Messenger> pMessenger;
		unique_ptr<MessageDispatch> pMessageDispatch;
		unique_ptr<EntityLoader> pEntityLoader;

		ComponentEntity* pRootEntity = nullptr;
		Scene* pScene = nullptr;
	};

	SystemsMode::SystemsMode(RegisterSystemsFn fnGameSysRegister) :
		m_pImpl(vnew(ALLOC_OBJECT)InternalData)
	{
		m_registerSystemsFn = fnGameSysRegister;
	}

	SystemsMode::~SystemsMode()
	{

	}

	void SystemsMode::CreateEntityHierarchy()
	{
		m_pImpl->pEntityLoader.reset(vnew(ALLOC_OBJECT)EntityLoader(*m_pImpl->pComponentManager, m_pImpl->pComponentManager->GetSystemCoordinator()));


		usg::EntitySpawnParams spawnParams;
		usg::Entity pRoot = m_pImpl->pEntityLoader.get()->SpawnEntityFromTemplate("Entities/StandardRoot.vent", nullptr, spawnParams);
		m_pImpl->pRootEntity = pRoot;

		auto pEntityLoaderHandle = GameComponents<EntityLoaderHandle>::Create(pRoot);
		pEntityLoaderHandle->pHandle = m_pImpl->pEntityLoader.get();

		auto pEventManagerHandle = GameComponents<EventManagerHandle>::Create(pRoot);
		pEventManagerHandle->handle = &m_pImpl->pComponentManager->GetEventManager();

		Required<SceneComponent> scene;
		GetComponent(pRoot, scene);
		ASSERT(scene.IsValid());
		m_pImpl->pScene = scene.GetRuntimeData().pScene;
	}

	void SystemsMode::Init(GFXDevice* pDevice, usg::ResourceMgr* pResMgr)
	{
		m_pImpl->pDevice = pDevice;
		m_pImpl->pComponentManager.reset(vnew(ALLOC_COMPONENT)ComponentManager(m_pImpl->pMessageDispatch.get(), m_registerSystemsFn));

		CreateEntityHierarchy();
		Required<ActiveDevice> activeDevice;
		GetComponent(GetRootEntity(), activeDevice);
		ActiveDevice_init(activeDevice, pDevice);

		m_pImpl->pComponentManager->RegisterResourceHandles(pDevice, pResMgr, m_pImpl->pScene);
		
	}

	void SystemsMode::Cleanup(GFXDevice* pDevice)
	{

	}

	bool SystemsMode::Update(float fElapsed)
	{
		if (m_pImpl->pComponentManager != nullptr)
		{
			m_pImpl->pComponentManager->TriggerAllSignals(fElapsed, m_pImpl->pDevice);
		}
		return false;
	}

	void SystemsMode::InitNetworking(UsagiNet& usagiNetwork)
	{
		m_pImpl->pNetManager = &usagiNetwork.GetNetworkManager();
		m_pImpl->pMessageDispatch.reset(vnew(ALLOC_OBJECT)MessageDispatch());
		m_pImpl->pMessenger.reset(vnew(ALLOC_OBJECT)Messenger(*m_pImpl->pMessageDispatch, m_pImpl->pNetManager));
	}

	ComponentManager* SystemsMode::GetComponentMgr()
	{
		return m_pImpl->pComponentManager.get();
	}

	ComponentEntity* SystemsMode::GetRootEntity()
	{
		return m_pImpl->pRootEntity;
	}

	Scene& SystemsMode::GetScene()
	{
		ASSERT(m_pImpl->pScene != nullptr);
		return *(m_pImpl->pScene);
	}
}