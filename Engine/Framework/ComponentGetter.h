/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// A utility to aid in getting components.

#ifndef _USG_COMPONENT_GETTER_H_
#define _USG_COMPONENT_GETTER_H_
#include "Engine/Framework/Component.h"

namespace usg
{

typedef class ComponentEntity* Entity;
class ComponentGetter
{
public:
	ComponentGetter(Entity e) : m_pEntity(e) {}

	Entity GetEntity() { return m_pEntity; }

	template<typename T>
	bool operator()(T& component);
	template<typename T1, typename T2>
	bool operator()(T1& component1, T2& component2);

private:

	template<typename T, typename SearchMask>
	bool GetComponentInt(Entity uEntity, Optional<T, SearchMask>& optional1, Optional<T, SearchMask>& optional2)
	{
		bool valid = GetComponentInt(uEntity, optional1);
		if (valid) { optional2 = optional1; }
		return valid;
	}

	template<typename T, typename SearchMask>
	bool GetComponentInt(Entity uEntity, Required<T, SearchMask>& required1, Required<T, SearchMask>& required2)
	{
		bool valid = GetComponentInt(uEntity, required1);
		if (valid) { required2 = required1; }
		return valid;
	}

	template<typename T, typename SearchMask>
	bool GetComponentInt(Entity uEntity, Optional<T, SearchMask>& optional, Required<T, SearchMask>& required)
	{
		Component<T>* c = ComponentGetterInt<T, SearchMask>::GetComponent(uEntity);

		bool exists = c != NULL;
		if (exists) { required = Required<T, SearchMask>(*c); }
		optional = Optional<T, SearchMask>(c);

		return exists;
	}

	template<typename T, typename SearchMask>
	bool GetComponentInt(Entity uEntity, Required<T, SearchMask>& required, Optional<T, SearchMask>& optional)
	{
		Component<T>* c = ComponentGetterInt<T, SearchMask>::GetComponent(uEntity);

		bool exists = c != NULL;
		if (exists) { required = Required<T, SearchMask>(*c); }
		optional = Optional<T, SearchMask>(c);

		return exists;
	}

	template<typename T, typename SearchMask>
	bool GetComponentInt(Entity uEntity, Optional<T, SearchMask>& optional)
	{
		optional = Optional<T, SearchMask>(ComponentGetterInt<T, SearchMask>::GetComponent(uEntity));
		return true;
	}

	template<typename T, typename SearchMask>
	bool GetComponentInt(Entity uEntity, Required<T, SearchMask>& required)
	{
		Component<T>* c = ComponentGetterInt<T, SearchMask>::GetComponent(uEntity);

		bool exists = c != NULL;
		if (exists) { required = Required<T, SearchMask>(*c); }

		return exists;
	}

	Entity m_pEntity;
};



template<typename T>
bool ComponentGetter::operator()(T& component)
{
	return GetComponentInt(m_pEntity, component);
}

template<typename T1, typename T2>
bool ComponentGetter::operator()(T1& component1, T2& component2)
{
	return GetComponentInt(m_pEntity, component1, component2);
}

}

#endif //_USG_COMPONENT_GETTER_H_
