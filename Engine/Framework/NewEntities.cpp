#include "Engine/Common/Common.h"
#include "Engine/Framework/ComponentEntity.h"
#include "Engine/Core/stl/set.h"
#include "Engine/Core/stl/deque.h"
#include "NewEntities.h"

namespace usg
{
	struct InternalData
	{
		deque<pair<ComponentEntity*, ComponentEntity*>> entities;
	};

	bool NewEntities::Contains(ComponentEntity* e) const
	{
		auto* pInternalData = static_cast<InternalData*>(m_pInternalData);
		for (auto it : pInternalData->entities)
		{
			if (it.first == e)
			{
				return true;
			}
		}
		return false;
	}

	pair<ComponentEntity*, ComponentEntity*> NewEntities::Pop()
	{
		auto* pInternalData = static_cast<InternalData*>(m_pInternalData);
		auto e = pInternalData->entities.front();
		pInternalData->entities.pop_front();
		return e;
	}
	
	bool NewEntities::Empty() const
	{
		auto* pInternalData = static_cast<InternalData*>(m_pInternalData);
		return pInternalData->entities.size() == 0;
	}

	void NewEntities::Add(ComponentEntity* e, ComponentEntity* parent)
	{
		auto* pInternalData = static_cast<InternalData*>(m_pInternalData);
		pInternalData->entities.emplace_back(e, parent);
	}

	NewEntities::NewEntities()
	{
		m_pInternalData = vnew(ALLOC_OBJECT)InternalData();
	}

	NewEntities::~NewEntities()
	{
		auto* pInternalData = static_cast<InternalData*>(m_pInternalData);
		vdelete pInternalData;
	}

}