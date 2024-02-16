/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/InstancedRenderer.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/ShadowContext.h"
#include "Engine/Graphics/Device/GFXContext.h"

namespace usg {

	void ShadowContextBase::Cleanup(GFXDevice* pDevice)
	{
		for (auto itr : m_instanceRenders)
		{
			itr.second->Cleanup(pDevice);
		}
	}

	void ShadowContextBase::ClearLists()
	{
		m_drawList.clear();
		Inherited::ClearLists();

		for (auto itr : m_instanceRenders)
		{
			itr.second->DrawFinished();
		}
	}

	void ShadowContextBase::CacheDirtyInfo()
	{
		usg::Matrix4x4 mLightTransform = GetLightMat();

		m_prevData.clear();
		for (auto itr : m_cmpList)
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

		if (m_prevData.size() != m_cmpList.size())
		{
			return true;
		}

		auto prev = m_prevData.begin();
		for(auto itr : m_cmpList)
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



	void ShadowContextBase::ReplaceInstancedNodes(usg::GFXDevice* pDevice)
	{
		// Now look for instances and build a new list
		InstancedRenderer* pCurrentInstanceRenderer = nullptr;
		uint64 uCurrentInstance = USG_INVALID_ID64;

		m_cmpList = m_drawList;

		for(auto itr = m_drawList.begin(); itr != m_drawList.end();)
		{
			RenderNode* pNode = (*itr);
			uint64 uInstanceId = pNode->GetInstanceId();
			if (pCurrentInstanceRenderer && uInstanceId != uCurrentInstance)
			{
				pCurrentInstanceRenderer = nullptr;
				uCurrentInstance = USG_INVALID_ID64;
			}

			if (uInstanceId != USG_INVALID_ID64)
			{
				if (uInstanceId == uCurrentInstance)
				{
					pCurrentInstanceRenderer->AddNode(pNode);
					itr = m_drawList.erase(itr);
				}
				else
				{
					if (m_instanceRenders.find(uInstanceId) == m_instanceRenders.end())
					{
						m_instanceRenders[uInstanceId] = pNode->CreateInstanceRenderer(pDevice, GetScene());
					}
					pCurrentInstanceRenderer = m_instanceRenders[uInstanceId];
					uCurrentInstance = uInstanceId;
					pCurrentInstanceRenderer->AddNode(pNode);
					itr = m_drawList.erase(itr);
				}
			}
			else
			{
				itr++;
			}
		}


		for (auto itr : m_instanceRenders)
		{
			RenderNode* pNode = itr.second->EndBatch();
			if (pNode)
			{
				m_drawList.push_back(pNode);
			}
		}

		for (auto itr : m_instanceRenders)
		{
			itr.second->PreDraw(pDevice);
		}
	}


}

