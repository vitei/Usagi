/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Base structures for behavior containers
*****************************************************************************/

#ifndef __USG_AI_COMPOSITE__
#define __USG_AI_COMPOSITE__
#include "Engine/Common/Common.h"
#include "Behavior.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class IComposite : public IBehavior<ContextType>
{
public:
	IComposite():
	m_uNumChildren(0),
	m_uSize(0),
	m_iPtrDiffToChildren(0) {}

	//	The memory position for children is guaranteed to be after the composite in the behavior tree array
	//	so we store the offset for the memory position in relation to this composite
	void AddChild(IBehavior<ContextType>* pChild)
	{
		ASSERT(this->m_uNumChildren < this->m_uSize);
		ptrdiff_t p = ((uintptr_t)pChild - (uintptr_t)this);
		uint16* pChildren = this->GetPointerToChildren();
		pChildren[this->m_uNumChildren++] = static_cast<uint16>(p);
	}
	
	IBehavior<ContextType>& GetChild(uint32 uIndex)
	{
		ASSERT(uIndex < this->m_uNumChildren);
		uint16* pChildren = this->GetPointerToChildren();
		return *(IBehavior<ContextType>*)((uintptr_t)this + pChildren[uIndex]);
	}

	void InitChunk(uint16* pChunk, uint16 uSize) 
	{ 
		m_iPtrDiffToChildren = (ptrdiff_t)((uint8*)pChunk - (uint8*)this);
		this->m_uSize = uSize;
	}

	uint16 GetNumChildren() const { return m_uNumChildren; }
	uint16 GetSize() const { return m_uSize; }

	virtual ~IComposite()
	{
		uint32 i, uC = this->m_uNumChildren;
		for (i = 0; i < uC; i++)
		{
			this->GetChild(i).~IBehavior();
		}
	}


protected:
	virtual void Shut(int iStatus)
	{
		uint32 i, uC = this->m_uNumChildren;
		for (i = 0; i < uC; i++)
		{
			this->GetChild(i).Shut(iStatus);
		}
	}
	uint16* GetPointerToChildren()
	{
		return (uint16*)((uintptr_t)this + m_iPtrDiffToChildren);
	}

	uint16 m_uNumChildren;
	uint16 m_uSize;
private:
	ptrdiff_t m_iPtrDiffToChildren;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_COMPOSITE__