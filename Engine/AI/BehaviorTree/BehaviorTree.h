/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIOR_TREE__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIOR_TREE__

#include "Engine/Memory/ScratchRaw.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/Variables.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class BehaviorTree
{
public:
	BehaviorTree():
		m_pBuffer(NULL),
		m_pRoot(NULL),
		m_fElapsed(0.0f),
		m_uTicks(0),
		m_uSize(0),
		m_uOffset(0),
		m_pVariables(NULL)
	{
		
	}

	~BehaviorTree()
	{
		Shut();
	}

	void Shut()
	{
		if(m_pBuffer)
		{
			m_pRoot->~IBehavior<ContextType>();
			vdelete[] m_pBuffer;
			m_pBuffer = NULL;
			m_uOffset = 0;
			m_uSize = 0;
			m_fElapsed = 0;
		}
		if (m_pVariables)
		{
			vdelete m_pVariables;
			m_pVariables = NULL;
		}
	}

	uint32 GetSize() const { return m_uSize; }
	uint32 GetOffset() const { return m_uOffset; }

	// Allocating memory for any type of behavior, composite or decorator
	template <class ElemType>
	ElemType* Alloc()
	{
		ASSERT((m_uOffset + (sizeof(ElemType))) < m_uSize);
		static const uint8 REQUIRED_ALIGNMENT = 4;

		// Manually aligning to the next 4-byte boundary because of errors
		// Is there a way to do this just by passing a parameter to new?
		uint8 current_alignment = m_uOffset % REQUIRED_ALIGNMENT;
		if(current_alignment != 0) { m_uOffset += REQUIRED_ALIGNMENT - current_alignment; }

		ElemType* pBehavior = new ((void*)((uintptr_t)m_pBuffer + m_uOffset)) ElemType();
		ASSERT(pBehavior != NULL);
		m_uOffset += sizeof(ElemType);
		return pBehavior;
	}

	//	Used to allocate dynamically sized chunks (usually uint16) for composites as their number of children is initially unknown. 
	//	These uint16 pointers are used to store children for composites.
	template <class ElemType>
	ElemType* AllocChunk(uint32 uSize)
	{
		ASSERT((m_uOffset + (sizeof(ElemType) * uSize)) < m_uSize);
		ElemType* pChunk = (ElemType*)(m_pBuffer + m_uOffset);
		ASSERT(pChunk != NULL);
		m_uOffset += sizeof(ElemType) * uSize;
		return pChunk;
	}

	//	This method takes the method pointer (Load) from the behavior factory, loads the tree and sets the appropriate start behavior
	template <class FactoryType>
	void Load(IBehavior<ContextType>* ((FactoryType::*fp)(void)), FactoryType* pFactoryObject)
	{
		ASSERT(m_pBuffer == NULL && m_uOffset == 0 && m_pVariables == NULL);

		m_pVariables = vnew(ALLOC_OBJECT)BehaviorTreeVariables();

		// Initialize temporary buffer (which should be large enough to accommodate any behavior tree)
		const uint32 uTempBufferSize = 1024*4;
		ScratchRaw tempBuf(uTempBufferSize, 4);
		m_pBuffer = (uint8*)tempBuf.GetRawData();
		m_uSize = uTempBufferSize;

		// Load the tree
		m_pRoot = (pFactoryObject->*fp)();

		// Memcpy to a new buffer and ensure that our pointers remain valid
		ptrdiff_t uRootPtrDiff = ((uintptr_t)m_pRoot - (uintptr_t)m_pBuffer);
		const uint32 uBehaviorTreeSize = m_uOffset;
		m_pBuffer = vnew(usg::ALLOC_BEHAVIOR_TREE) uint8[uBehaviorTreeSize];
		memcpy(m_pBuffer, tempBuf.GetRawData(), uBehaviorTreeSize);
		m_uSize = uBehaviorTreeSize;
		m_pRoot = (IBehavior<ContextType>*)(m_pBuffer + uRootPtrDiff);
	}

	bool WillTick(float fElapsed, uint32 uTargetFrames) const
	{
		const float fExecInterval = uTargetFrames / 30.0f;
		return m_fElapsed + fElapsed >= fExecInterval;
	}

	void Advance(float fElapsed)
	{
		m_fElapsed += fElapsed;
	}

	bool Tick(float fElapsed, ContextType& context, uint32 uTargetFrames)
	{
		const float fExecInterval = uTargetFrames / 30.0f;
		Advance(fElapsed);
		if(m_fElapsed >= fExecInterval)
		{
			m_uTicks++;
			m_pRoot->Tick(m_fElapsed, context);
			m_fElapsed = 0.0f;
			return true;
		}

		return false;
	}

	BehaviorTreeVariables* GetVariables()
	{
		return m_pVariables;
	}

private:
	uint8* m_pBuffer;
	IBehavior<ContextType>* m_pRoot;
	float m_fElapsed;
	uint32 m_uTicks;
	uint32 m_uSize;
	uint32 m_uOffset;
	BehaviorTreeVariables* m_pVariables;
};

}	// namespace ai

}	// namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIOR_TREE__
