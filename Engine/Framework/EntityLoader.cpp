#include "Engine/Common/Common.h"
#include "Engine/Framework/EntityLoader.h"
#include "Engine/Framework/EntityLoaderHandle.h"
#include "Engine/Framework/ComponentManager.h"

namespace usg
{

	template<>
	void RegisterComponent<usg::Components::EntityLoaderHandle>(SystemCoordinator& systemCoordinator)
	{
		systemCoordinator.RegisterComponent<::usg::Components::EntityLoaderHandle>();
	}


	EntityLoader::EntityLoader(ComponentManager& componentManager, SystemCoordinator& systemCoordinator) :
		m_componentManager(componentManager),
		m_systemCoordinator(systemCoordinator)
	{

	}

	void  EntityLoader::ApplyTemplateToEntity(const char* szFilename, Entity root)
	{
		m_componentManager.ApplyTemplateToEntity(szFilename, root);
	}

	ComponentEntity* EntityLoader::SpawnEntityFromTemplate(const char* szFilename, ComponentEntity* parent, const EntitySpawnParams& spawnParams)
	{
		return m_componentManager.SpawnEntityFromTemplate(szFilename, parent, spawnParams);
	}

	void EntityLoader::ForceCallOnLoaded(Entity e)
	{
		m_componentManager.ForceCallOnLoaded(e);
	}
}
