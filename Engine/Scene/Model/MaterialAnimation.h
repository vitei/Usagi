/****************************************************************************
//	Usagi Engine, Copyright Vitei, Inc. 2013
//	Description: Tracks the current frame, the meshes and materials that are impacted
*****************************************************************************/
#ifndef _USG_SCENE_MODEL_MATERIAL_ANIMATION_H_
#define _USG_SCENE_MODEL_MATERIAL_ANIMATION_H_

#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Resource/MaterialAnimationResource.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Scene/Model/AnimationBase.h"

namespace usg {

class Model;
class ModelResource;


class MaterialAnimation : public AnimationBase
{
	typedef AnimationBase Inherited;
public:
	MaterialAnimation();
	virtual ~MaterialAnimation();

	bool Init(const char* szAnimName);
	
	void Update(float fElapsed);

	const string& GetName() { return m_name; }

	void ApplyToModel(Model& model);

private:
	string							m_name;
	MaterialAnimationResHndl		m_pAnimResource;
};

}

#endif	

