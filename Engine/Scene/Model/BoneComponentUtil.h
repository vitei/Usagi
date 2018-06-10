/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//  Description: Utility functions for initializing bone components
****************************************************************************/
#ifndef _USG_BONE_COMPONENT_UTIL_H_
#define _USG_BONE_COMPONENT_UTIL_H_

#include "Engine/Framework/ComponentEntity.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"
#include "Engine/Scene/Model/Skeleton.h"

// Mention the Components namespace so the boilerplate tool
// will add this header to the list of includes
namespace Components {}

namespace usg
{

void InitializeBones(Entity e, ComponentLoadHandles& handles, bool bWasPreviouslyCalled);
void InitializeMatrix(Entity e, ComponentLoadHandles& handles);
void InitializeScale(Entity e, ComponentLoadHandles& handles);
void SetBoneTransforms(Entity e, ComponentLoadHandles& handles);
void AddMatrixToEntity(Entity e, ComponentLoadHandles& handles);
void AddScaleToEntity(Entity e, ComponentLoadHandles& handles);

// This is a convenience function which calls the four functions
// listed above.
void ProcessBoneEntity(Entity e, ComponentLoadHandles& handles, bool bWasPreviouslyCalled);
void ProcessIntermediateBoneEntity(Entity e, ComponentLoadHandles& handles, bool bWasPreviouslyCalled);

// This functor is for manually re-initializing the model component
// after you've called ClearModel on it.
class BoneFunctor
{
public:
	BoneFunctor() {}

	inline void operator()(Entity e, ComponentLoadHandles& handles)
	{
		ProcessBoneEntity(e, handles, false);
	}
};


// This one for updating its matrices
class UpdateBoneModelMat
{
public:
	UpdateBoneModelMat() {}

	inline void operator()(Entity e, ComponentLoadHandles& handles)
	{
		usg::ProcessBoneEntity(e, handles, true);
	}
};

template<> void OnLoaded<IntermediateBone>(Component<IntermediateBone>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled);
template<> void OnLoaded<BoneComponent>(Component<BoneComponent>& c, ComponentLoadHandles& handles, bool bWasPreviouslyCalled);
template<> void OnActivate<BoneComponent>(Component<BoneComponent>& c);

}

#endif
