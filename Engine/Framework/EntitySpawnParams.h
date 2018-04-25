#ifndef ENTITY_SPAWN_PARAMS
#define ENTITY_SPAWN_PARAMS
#include "Engine/Common/Common.h"
#include "Engine/Framework/FrameworkComponents.pb.h"

namespace usg
{

	class EntitySpawnParams
	{
	public:
		EntitySpawnParams() { m_uOverrideFlags = 0; m_iNuid = 0; m_iOwnerNuid = 0; }
		~EntitySpawnParams() {}

		void SetNUID(sint64 uNuid) { m_iNuid = uNuid; m_uOverrideFlags |= SET_NUID; }
		void SetOwnerNUID(sint64 uNuid) { m_iOwnerNuid = uNuid; m_uOverrideFlags |= SET_OWNER_NUID; }
		void SetTransform(const TransformComponent& trans) { m_transform = trans; m_uOverrideFlags |= SET_TRANSFORM; }

		bool HasNUID() const { return m_uOverrideFlags & SET_NUID; }
		bool HasOwnerNUID() const { return m_uOverrideFlags & SET_OWNER_NUID; }
		bool HasTransform() const { return m_uOverrideFlags & SET_TRANSFORM; }

		sint64 GetNUID() const { ASSERT(HasNUID()); return m_iNuid; }
		sint64 GetOwnerNUID() const { ASSERT(HasOwnerNUID()); return m_iOwnerNuid; }
		const TransformComponent& GetTransform() const { ASSERT(HasTransform()); return m_transform; }

	private:
		enum Params
		{
			SET_TRANSFORM = (1 << 0),
			SET_NUID = (1 << 1),
			SET_OWNER_NUID = (1 << 2)
		};

		uint32				m_uOverrideFlags;
		sint64				m_iNuid;
		sint64				m_iOwnerNuid;
		TransformComponent	m_transform;
	};
}

#endif