/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Model/AnimationChain.pb.h"
#include "MaterialAnimation.h"

namespace usg
{

	MaterialAnimation::MaterialAnimation()
	{
		Inherited::Reset();
	}

	MaterialAnimation::~MaterialAnimation()
	{
		
	}


	bool MaterialAnimation::Init(const char* szAnimName)
	{
		U8String fileName = szAnimName;
		fileName += +".vmata";
		m_name = szAnimName;
		m_pAnimResource = ResourceMgr::Inst()->GetMaterialAnimation(fileName.CStr());
		if (!m_pAnimResource)
			return false;

		// For now setting to always play
		Play();

		return true;

	}

	void MaterialAnimation::Update(float fElapsed)
	{
		UpdateInt(fElapsed, m_pAnimResource->GetFrameRate(), (float)m_pAnimResource->GetFrameCount(), m_pAnimResource->IsLoop());
	}

	void MaterialAnimation::ApplyToModel(Model& model)
	{
		m_pAnimResource->ApplyAllMaterialAnimations(m_fActiveFrame, model);
	}

}