/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Component.h"

using namespace usg;

ComponentType* ComponentType::GetNextComponent() const
{
	return m_pNextComponent;
}

void ComponentType::SetNextComponent(ComponentType* pComp)
{
	ASSERT(pComp != m_pPrevComponent || (pComp == NULL && m_pPrevComponent == NULL));
	m_pNextComponent = pComp;
}

ComponentType* ComponentType::GetPrevComponent() const
{
	return m_pPrevComponent;
}

void ComponentType::SetPrevComponent(ComponentType* pComp)
{
	ASSERT(pComp != m_pNextComponent || (pComp == NULL && m_pNextComponent == NULL));
	m_pPrevComponent = pComp;
}

// TODO: Make this thread-safe
uint32 ComponentType::GetNextTypeID()
{
	static uint32 s_uNextTypeID = 0;
	return s_uNextTypeID++;
}
