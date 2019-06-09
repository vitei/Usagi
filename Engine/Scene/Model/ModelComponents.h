#ifndef __ENGINE_SCENE_MODEL_MODEL_COMPONENTS_H__
#define __ENGINE_SCENE_MODEL_MODEL_COMPONENTS_H__

#include "Engine/Framework/Component.h"
#include "Engine/Resource/ResourceDecl.h"

namespace usg
{
	class ModelAnimPlayer;
	class Bone;
	class Model;
	
	template<>
	struct RuntimeData<usg::Components::BoneComponent>
	{
		Bone* pBone;
	};

	template<>
	struct RuntimeData<usg::Components::ModelComponent>
	{
		Model* pModel;
	};

	template<>
	struct RuntimeData<usg::Components::ModelAnimComponent>
	{
		usg::ModelAnimPlayer*	pAnimPlayer;
	};

	void InitModel(Component<ModelComponent>& p, ComponentLoadHandles& handles);
	void ClearModel(Component<ModelComponent>& p, ComponentLoadHandles& handles);

	template<>
	void PreloadComponentAssets<ModelComponent>(const usg::ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles);

	template<>
	void PreloadComponentAssets<ModelAnimComponent>(const usg::ComponentHeader& hdr, ProtocolBufferFile& file, ComponentLoadHandles& handles);

}


#endif