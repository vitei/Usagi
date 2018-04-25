/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// A utility to aid in getting components.

#ifndef _USG_COMPONENT_GETTER_H_
#define _USG_COMPONENT_GETTER_H_

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
	Entity m_pEntity;
};

template<typename T>
bool ComponentGetter::operator()(T& component)
{
	return GetComponent(m_pEntity, component);
}

template<typename T1, typename T2>
bool ComponentGetter::operator()(T1& component1, T2& component2)
{
	return GetComponent(m_pEntity, component1, component2);
}

}

#endif //_USG_COMPONENT_GETTER_H_
