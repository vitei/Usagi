/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A basic renderable element
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_RENDER_NODE_H_
#define _USG_GRAPHICS_SCENE_RENDER_NODE_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Scene/RenderGroup.h"

namespace usg{

class Scene;
class TransformNode;
class RenderGroup;
class GFXContext;
class PostFXSys;
class Texture;
class InstancedRenderer;

class RenderNode
{
public:
	RenderNode();
	virtual ~RenderNode();

	enum ComparisonShift
	{
		COMPARISON_SHIFT_PRIORITY = 52,
		COMPARISON_SHIFT_TEXTURE = 0,
		COMPARISON_SHIFT_EFFECT = 8,
	};

	enum RenderPass
	{
		RENDER_PASS_DEFERRED = 0,
		RENDER_PASS_FORWARD,
		RENDER_PASS_DEPTH,
		RENDER_PASS_DEPTH_OMNI
	};

	struct RenderContext
	{
		RenderContext() {}
		const class DescriptorSet* pGlobalDescriptors = nullptr;
		PostFXSys* pPostFX = nullptr;
		RenderPass eRenderPass = RENDER_PASS_FORWARD;
	};

	void SetParent(RenderGroup* pParent);
	void SetLayer(RenderLayer eLayer);
	void SetPriority(uint8 uPriority) { m_uPriority = uPriority; SetPriorityCmpValue(); }
	void SetRenderMask(uint32 uMask);
	void SetRenderMaskIncShadow(uint32 uMask);
	uint32 GetRenderMask() const { return m_uRenderMask; }
	bool IsAnimated() const { return m_bAnimated;  }
	void SetAnimated(bool bAnimated) { m_bAnimated = bAnimated; }

	RenderLayer	GetLayer() const { return m_eLayer; }
	uint8	GetPriority() const { return m_uPriority; }


	bool operator<(const RenderNode &rhs) const;
	bool operator>(const RenderNode &rhs) const;

	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext) { ASSERT(false); return false; }
	// Should be overloaded, but not necessary for post process effects not ever assigned to a render group
	virtual void RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass, const SceneRenderPasses& passes) {} // { ASSERT(false); }
	virtual void VisibilityUpdate(GFXDevice* pDevice, const Vector4f& vTransformOffset) {}
	virtual uint64 GetInstanceId() const { return USG_INVALID_ID64; }
	virtual InstancedRenderer* CreateInstanceRenderer(GFXDevice* pDevice, Scene* pScene) { return nullptr; }

	bool GetPostEffect() { return m_bPostEffect; }
	bool HasShadow() { return m_bHasShadow; }

	void SetMaterialCmpVal(const PipelineStateHndl& hndl , const Texture* pTexture0);

	const RenderGroup* GetParent() const { return m_pParent; }

	float GetDepthSortValue() const { return m_pParent->GetSortingDistance(); }
	uint64 GetComparisonValue() const { return m_uComparisonVal; }

protected:
	void SetPostEffect(bool bPostEffect) { m_bPostEffect = bPostEffect; }
	void SetHasShadow(bool bShadow);
	void SetPriorityCmpValue();

private:
	PRIVATIZE_COPY(RenderNode);
	RenderGroup*			m_pParent;
	RenderLayer				m_eLayer;
	uint8					m_uPriority;
	uint32					m_uRenderMask;
	bool					m_bPostEffect;
	bool					m_bHasShadow;
	bool					m_bAnimated;
	uint64					m_uComparisonVal;
};

inline bool RenderNode::operator<(const RenderNode &rhs) const
{
	return (m_uComparisonVal < rhs.m_uComparisonVal);
}

inline bool RenderNode::operator>(const RenderNode &rhs) const
{
	return (m_uComparisonVal > rhs.m_uComparisonVal);
}

}


#endif

