/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _COMPONENT_SYSTEM_INPUT_OUTPUTS_H
#define _COMPONENT_SYSTEM_INPUT_OUTPUTS_H


#include "ComponentEntity.h"
#include "Engine/Framework/SystemId.h"

namespace usg {

static const size_t SYSTEM_DEFAULT_POOL_SIZE = 100;

struct GenericInputOutputs : public HierearchyNode<GenericInputOutputs>
{
	GenericInputOutputs(void* inputs, void* outputs)
		: entity(nullptr), genericInputs(inputs), genericOutputs(outputs) {}
	Entity      entity;
	void* const genericInputs;
	void* const genericOutputs;
};

class ComponentSystemInputOutputsSharedBase
{
public:
	static void Cleanup();
	static GenericInputOutputs* GetRootSystem(uint32 uSystemId);
protected:
	static void InitBase(uint32 uSystemId, uint32 uElementSize, uint32 uGroupSize, GenericInputOutputs* pRootNode);
	static void CleanupBase(uint32 uSystemId);
	static void* Alloc(uint32 uSystemId);
	static void Free(uint32 uSystemId, void* pIO);
	static void AttachGenericInputOutputs(Entity e, GenericInputOutputs* pIO, GenericInputOutputs* pRoot, uint32 uSystemId);
	static void RemoveGenericInputOutputs(Entity e, uint32 uSystemId);
};

template<typename S>
class ComponentSystemInputOutputs : public ComponentSystemInputOutputsSharedBase
{
public:
	struct InputOutputs : public GenericInputOutputs
	{
		InputOutputs() : GenericInputOutputs(&inputs, &outputs) {}
		typename S::Inputs  inputs;
		typename S::Outputs outputs;
	};

	static void Init(uint32 uPoolSize=SYSTEM_DEFAULT_POOL_SIZE)
	{
		mData.uTypeID = GetSystemId<S>();
		InitBase(mData.uTypeID, (uint32)sizeof(InputOutputs), SYSTEM_DEFAULT_POOL_SIZE, &mData.rootNode);
	}

	static void Cleanup()
	{
		mData.rootNode.ClearHierarchy();
		CleanupBase(mData.uTypeID);
	}

	static void AddInputOutputs(Entity e, typename S::Inputs& inputs, typename S::Outputs& outputs);
	static void UpdateInputOutputs(Entity e, typename S::Inputs& inputs, typename S::Outputs& outputs);
	static void RemoveInputOutputs(Entity e);

	static uint32 TypeID() { return mData.uTypeID; }

private:
	struct Data {
		Data()
		: uTypeID(0)
		{}

		uint32                  uTypeID;
		InputOutputs			rootNode;	// No data, just used to traverse the hierarchy
	};
	

	static Data         mData;
};

// Global functions
// These functions are provided to give easy access to a System's methods
// through C++'s template type inference.

template<typename S>
void InitSystem(uint32 uPoolSize=SYSTEM_DEFAULT_POOL_SIZE)
{
	ComponentSystemInputOutputs<S>::Init(uPoolSize);
}

template<typename S>
void CleanupSystem()
{
	ComponentSystemInputOutputs<S>::Cleanup();
}

template<typename S>
bool UpdateInputOutputs(ComponentGetter& componentGetter, bool bEntityHasRequiredComponents, bool bEntityIsCurrentlyRunning)
{
	Entity e = componentGetter.GetEntity();
	typename S::Inputs inputs;
	typename S::Outputs outputs;

	if(bEntityIsCurrentlyRunning)
	{
		if (bEntityHasRequiredComponents && S::GetInputOutputs(componentGetter, inputs, outputs))
		{
			ComponentSystemInputOutputs<S>::UpdateInputOutputs(e, inputs, outputs);
			return true;
		}
		else
		{
			ComponentSystemInputOutputs<S>::RemoveInputOutputs(e);
		}
	}
	else
	{
		if (bEntityHasRequiredComponents && S::GetInputOutputs(componentGetter, inputs, outputs))
		{
			ComponentSystemInputOutputs<S>::AddInputOutputs(e, inputs, outputs);
			return true;
		}
	}
	return false;
}

template<typename S>
void ComponentSystemInputOutputs<S>::AddInputOutputs(Entity e, typename S::Inputs& inputs, typename S::Outputs& outputs)
{
	InputOutputs* io = new(Alloc(mData.uTypeID))InputOutputs();
	
	io->inputs = inputs;
	io->outputs = outputs;
	AttachGenericInputOutputs(e, io, &mData.rootNode, GetSystemId<S>());
}

template<typename S>
void ComponentSystemInputOutputs<S>::UpdateInputOutputs(Entity e, typename S::Inputs& inputs, typename S::Outputs& outputs)
{
	InputOutputs* io = (InputOutputs*) e->GetSystem(mData.uTypeID);

	io->entity = e;
	io->inputs = inputs;
	io->outputs = outputs;
}

template<typename S>
void ComponentSystemInputOutputs<S>::RemoveInputOutputs(Entity e)
{
	RemoveGenericInputOutputs(e, mData.uTypeID);
}

//Define the actual datastore -- this is statically defined and will be instantiated
// the first time we access a ComponentSystemInputOutputs of that type!
template<typename T>
typename ComponentSystemInputOutputs<T>::Data ComponentSystemInputOutputs<T>::mData;

}

#endif //_COMPONENT_SYSTEM_INPUT_OUTPUTS_H
