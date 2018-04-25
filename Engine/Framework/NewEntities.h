/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#pragma  once
#include "Engine/Core/stl/utility.h"

namespace usg
{
	class NewEntities
	{
	public:
		bool Contains(ComponentEntity* e) const;
		bool Empty() const;
		void Add(ComponentEntity* e, ComponentEntity* parent);
		pair<ComponentEntity*, ComponentEntity*> Pop();
		NewEntities();
		~NewEntities();
		NewEntities(const NewEntities&) = delete;
		NewEntities& operator=(const NewEntities&) = delete;
	private:
		void* m_pInternalData;
	};
}