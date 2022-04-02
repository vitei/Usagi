#pragma once

namespace usg
{
	class ComponentManager;
	class SystemCoordinator;
	class ComponentEntity;
	class EntitySpawnParams;
	
	namespace Components
	{
		typedef struct _TransformComponent TransformComponent;
	}

	class EntityLoader
	{
	public:
		EntityLoader(ComponentManager& componentManager, SystemCoordinator& systemCoordinator);

		// Spawn an entity from a .yml file.
		ComponentEntity* SpawnEntityFromTemplate(const char* szFilename, ComponentEntity* parent, const EntitySpawnParams& spawnParams);

		void ApplyTemplateToEntity(const char* szFilename, ComponentEntity* root);
		void ForceCallOnLoaded(Entity e);
	private:
		ComponentManager& m_componentManager;
		SystemCoordinator& m_systemCoordinator;
	};
}