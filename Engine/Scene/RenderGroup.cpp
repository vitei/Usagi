/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Scene/Scene.h"
#include "RenderGroup.h"
#include <float.h>

namespace usg {

const float LOD_NO_CULLING = FLT_MAX;
const float LOD_TOLERANCE = 1.02f;


RenderGroup::RenderGroup()
{
	m_uLODGroups = 0;
	m_pTransform = NULL;
	m_uRenderMask = 0;
	m_bPrevValid = false;
	m_uLastUpdate = 0;
	m_bVisiblityUpdate = false;
	m_bViewDistanceUpdate = false;
	m_viewData.fSortDistance = 0.0f;
	m_viewData.uPrevLOD = 0;
	m_pScene = NULL;
	m_bSort = false;
}


RenderGroup::~RenderGroup()
{

}

void RenderGroup::Init(const TransformNode* pTransform, Scene* pScene)
{
	m_uLastUpdate = pScene->GetFrame();
	for(uint32 i=0; i<MAX_LOD_GROUPS; i++)
	{
		for (int i = 0; i < MAX_LOD_GROUPS; i++)
		{
			m_lodGroups[i].nodes.clear();
		}
		m_lodGroups[i].fMaxDistanceSq	= LOD_NO_CULLING;
	}

	m_pTransform = pTransform;	
	m_pScene = pScene;
	m_bLODCulling = false;
	m_bVisiblityUpdate = false;
	m_bViewDistanceUpdate = false;
	m_viewData.fSortDistance = 0.0f;
	m_viewData.uPrevLOD = 0;
	m_bSort = pTransform != NULL;
}

void RenderGroup::Cleanup()
{
	m_uLODGroups	= 0;
	m_pTransform	= NULL;
	m_uRenderMask	= 0;
	m_bPrevValid	= false;
	m_bVisiblityUpdate = false;
	m_bViewDistanceUpdate = false;
	m_viewData.fSortDistance = 0.0f;
	m_viewData.uPrevLOD = 0;
	m_bSort = false;

	for(int i=0; i<MAX_LOD_GROUPS; i++)
	{
		m_lodGroups[i].nodes.clear();		
	}
}



void RenderGroup::AddRenderNodes(RenderNode** ppNodes, uint32 uCount, uint32 uLod)
{
	ASSERT(uLod < MAX_LOD_GROUPS);
	LODGroup &lodGroup = m_lodGroups[uLod];

	for(uint32 i=0; i<uCount; i++)
	{
		ppNodes[i]->SetParent(this);
		// FIXME: Needs a callback if the layer changes too
		if (ppNodes[i]->GetLayer() == RenderNode::LAYER_TRANSLUCENT)
		{
			m_bViewDistanceUpdate = true;
		}
		lodGroup.nodes.push_back(ppNodes[i]);
	}

	
	if(uLod >= m_uLODGroups)
	{
		m_uLODGroups = uLod+1;
	}

	UpdateMask();
}

void RenderGroup::UpdateMask()
{
	uint32 uPrevMask = m_uRenderMask;
	m_uRenderMask = 0;
	for(uint32 uLod =0; uLod<m_uLODGroups; uLod++)
	{
		LODGroup &lodGroup = m_lodGroups[uLod];
		for (auto it : lodGroup.nodes)
		{
			m_uRenderMask |= it->GetRenderMask();
		}
	}

	if(uPrevMask != m_uRenderMask && m_pTransform)
	{
		m_pScene->UpdateMask(m_pTransform, m_uRenderMask);
	}
}

void RenderGroup::SetLodMaxDistance(uint32 uLod, float fMaxDistance)
{
	ASSERT(uLod < MAX_LOD_GROUPS);
	m_lodGroups[uLod].fMaxDistanceSq = fMaxDistance*fMaxDistance;
	m_bLODCulling = true;
}


void RenderGroup::VisibilityUpdate(GFXDevice* pDevice)
{
	for (uint32 uLOD = 0; uLOD < m_uLODGroups; uLOD++)
	{
		for (auto it : m_lodGroups[uLOD].nodes)
		{
			Vector4f vDummy(0.0f, 0.0f, 0.0f, 0.0f);
			it->VisibilityUpdate(pDevice, vDummy);
		}
	}
}

bool RenderGroup::GetLodInt(const Vector4f &cameraPos, uint32& lodOut, float fLODBias)
{
	if(m_pTransform)
	{
		ASSERT(usg::Math::IsEqual(cameraPos.w, 1.0f));
		float distanceSq = m_pTransform->GetMatrix().vPos().GetSquaredDistanceFrom( cameraPos );
		for(uint32 i=0; i<m_uLODGroups; i++)
		{
			float fMaxDistanceSq = m_lodGroups[i].fMaxDistanceSq * fLODBias;
			if (i == m_viewData.uPrevLOD)
				fMaxDistanceSq *= LOD_TOLERANCE;	// To avoid flickering on and off


			if( distanceSq < (fMaxDistanceSq) )
			{
				m_viewData.uPrevLOD = i;
				lodOut = i;
				return true;
			}
		}
	}
	else
	{
		// We're not a 3D node
		lodOut = 0;
		return true;
	}

	return false;
}

bool RenderGroup::DrawnLastFrame() const
{
	return m_uLastUpdate == m_pScene->GetFrame();
}


void RenderGroup::RemoveRenderNode(RenderNode* pNode)
{
	for(sint32 i= m_uLODGroups-1; i>=0; i--)
	{
		LODGroup* pGroup = &m_lodGroups[i];

		for (usg::vector<RenderNode*>::reverse_iterator it = pGroup->nodes.rbegin(); it != pGroup->nodes.rend(); ++it)
		{
			if (*it == pNode)
			{
				pGroup->nodes.erase_unsorted(it);
			}
		}

		if (pGroup->nodes.empty())
		{
			m_uLODGroups = i;
		}
	}
}

bool RenderGroup::IsEmpty() const
{
	return m_uLODGroups == 0;
}

}

