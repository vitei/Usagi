/****************************************************************************
//	Usagi Engine, Copyright c Vitei, Inc. 2013
//	Description: A group of materials sharing the same transforms
//	and bounding area
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_RENDER_GROUP_H_
#define _USG_GRAPHICS_SCENE_RENDER_GROUP_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Core/stl/Vector.h"

namespace usg{

class TransformNode;
class Scene;
class RenderNode;
class VertexBuffer;
class IndexBuffer;

class RenderGroup
{
public:
	RenderGroup();
	~RenderGroup();

	void Init(const TransformNode* pNode, Scene* pScene);	// To be called by scene only
	void Cleanup();
	void AddRenderNodes(RenderNode** pNode, uint32 uCount, uint32 uLod = 0);
	void AddRenderNode(RenderNode* pNode, uint32 uLod = 0) { AddRenderNodes(&pNode, 1, uLod); }
	void RemoveRenderNode(RenderNode* pNode);
	bool IsEmpty() const;
	void SetLodMaxDistance(uint32 uLod, float fMaxDistance);
	bool GetLod(const Vector4f &cameraPos, uint32& lodOut, float fLODBias);
	RenderNode* GetLODRenderNode(uint32 uLOD, uint32 uNodeId) const;
	uint32 GetLODEntryCount(uint32 uLOD) const;
	bool HasTransform() const { return m_pTransform != NULL; }
	uint32 GetRenderMask() { return m_uRenderMask; }
	void UpdateMask();
	void UseVisibilityUpdate(bool bUpdate) { m_bVisiblityUpdate = bUpdate; }
	const TransformNode* GetTransform() const { return m_pTransform; }

	bool DrawnLastFrame() const;
	uint32 GetLastUpdateFrame() const { return m_uLastUpdate; }
	void VisibilityFunc(GFXDevice* pDevice, const Matrix4x4& mViewMat);

	enum
	{
		MAX_LOD_GROUPS = 3
	};

	// Only call manually if you don't have a transform node
	void SetSortPos(const Vector3f &vSortPos) { ASSERT(!m_pTransform); m_vSortPos = vSortPos; m_bSort = true; }
	// TODO: Add one per context
	float GetSortingDistance() const { return m_viewData.fSortDistance; }
private:
	bool GetLodInt(const Vector4f& cameraPos, uint32 &lodOut, float fLODBias=1.0f);
	void VisibilityUpdate(GFXDevice* pDevice);

	struct LODGroup
	{
		usg::vector<RenderNode*> nodes;
		float32					 fMaxDistanceSq;
	};

	struct ViewCtxtData
	{
		float				fSortDistance;
		uint32				uPrevLOD;
	};

	Scene*					m_pScene;
	const TransformNode*	m_pTransform;
	uint32					m_uLastUpdate;
	Vector3f				m_vSortPos;
	// FIXME: One of these per view
	ViewCtxtData			m_viewData;
	
	bool					m_bPrevValid;
	bool					m_bLODCulling;
	bool					m_bVisiblityUpdate;
	bool					m_bViewDistanceUpdate;
	bool					m_bSort;
	LODGroup				m_lodGroups[MAX_LOD_GROUPS];
	uint32					m_uLODGroups;
	uint32					m_uRenderMask;
};

inline RenderNode* RenderGroup::GetLODRenderNode(uint32 uLOD, uint32 uNodeId) const
{
	ASSERT(uLOD < m_uLODGroups);
	
	return m_lodGroups[uLOD].nodes[uNodeId];
}

inline uint32 RenderGroup::GetLODEntryCount(uint32 uLOD) const
{
	ASSERT(uLOD < m_uLODGroups);
	return (uint32)m_lodGroups[uLOD].nodes.size();
}

inline bool RenderGroup::GetLod(const Vector4f &cameraPos, uint32& lodOut, float fLODBias)
{
	if(!m_bLODCulling)
	{
		lodOut = 0;
		return m_uLODGroups > 0;
	}
	else
	{
		return GetLodInt(cameraPos, lodOut, fLODBias);
	}
}

inline void RenderGroup::VisibilityFunc(GFXDevice* pDevice, const Matrix4x4& mViewMat)
{
	if (m_bVisiblityUpdate)
	{
		VisibilityUpdate(pDevice);
	}

	if(m_bViewDistanceUpdate)
	{
		if(m_pTransform)
		{
			m_vSortPos = m_pTransform->GetMatrix().vPos().v3();
		}

		if (m_bSort)
		{
			m_viewData.fSortDistance = m_vSortPos.x*mViewMat._13 + m_vSortPos.y*mViewMat._23 + m_vSortPos.z*mViewMat._33 + mViewMat._43;
		}
	}
}

}


#endif
