/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/ShadowContext.h"
#include "Engine/Graphics/Device/GFXContext.h"

namespace usg {

	void ShadowContextBase::ClearLists()
	{
		m_drawList.clear();
		Inherited::ClearLists();
	}

	void ShadowContextBase::CacheDirtyInfo()
	{
		usg::Matrix4x4 mLightTransform = GetLightMat();

		m_prevData.clear();
		for (auto itr : m_drawList)
		{
			ComparisonData Cmp;
			Cmp.CmpNode = (memsize)itr;
			Cmp.CmpLoc = itr->GetParent()->GetTransform()->GetMatrix() * mLightTransform;
			m_prevData.push_back(Cmp);
		}
	}
	 
	bool ShadowContextBase::IsDirty() const
	{
		usg::Matrix4x4 mLightTransform = GetLightMat();

		if (m_prevData.size() != m_drawList.size())
		{
			return true;
		}

		auto prev = m_prevData.begin();
		for(auto itr : m_drawList)
		{
			if ((memsize)itr != prev->CmpNode)
			{
				return true;
			}

			// Disabling for now due to false positives, need to exclude external items
			/*if (itr->IsAnimated())
			{
				return true;
			}*/

			usg::Matrix4x4 CmpMat = itr->GetParent()->GetTransform()->GetMatrix() * mLightTransform;
			if (!CmpMat.IsEqual(prev->CmpLoc, FLT_EPSILON * 10.f))
			{
				return true;

			}
			++prev;
		}
		return false;
	}


}

