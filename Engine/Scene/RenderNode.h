/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A basic renderable element
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_RENDER_NODE_H_
#define _USG_GRAPHICS_SCENE_RENDER_NODE_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "Engine/Scene/RenderGroup.h"

namespace usg{

class Scene;
class TransformNode;
class RenderGroup;
class GFXContext;
class PostFXSys;
class Texture;

class RenderNode
{
public:
	RenderNode();
	virtual ~RenderNode();

	enum Layer
	{
		LAYER_BACKGROUND = 0,	// Planets, suns, etc
		LAYER_PRE_WORLD,
		LAYER_OPAQUE,
		LAYER_DEFERRED_SHADING,
		LAYER_SKY,	// Not lit, no point running it through the deferred shading
		LAYER_TRANSLUCENT,
		LAYER_SUBTRACTIVE,
		LAYER_ADDITIVE,
		LAYER_POST_PROCESS,
		LAYER_OVERLAY,
		LAYER_COUNT
	};

	enum RenderMask
	{
		RENDER_MASK_WORLD			= (1<<0),
		RENDER_MASK_LIGHTING		= (1<<1),
		RENDER_MASK_WATER			= (1<<2),
		RENDER_MASK_WORLD_EFFECT	= (1<<3),
		RENDER_MASK_POST_EFFECT		= (1<<4),
		RENDER_MASK_INSIDE			= (1<<5),
		RENDER_MASK_OUTSIDE			= (1<<6),
		RENDER_MASK_SHADOW_CAST		= (1<<7),
		RENDER_MASK_CUSTOM			= (1<<8),

		// Have all be everything except the shadow casting geometry
		RENDER_MASK_ALL				= (0xFFFFFFFF&(~RENDER_MASK_SHADOW_CAST)),
		RENDER_MASK_NONE			= 0
	};

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
		RenderContext() : pPostFX(nullptr), eRenderPass(RENDER_PASS_FORWARD) {}
		const class DescriptorSet* pGlobalDescriptors;
		PostFXSys* pPostFX;
		RenderPass eRenderPass;
	};

	void SetParent(RenderGroup* pParent);
	void SetLayer(Layer eLayer) { ASSERT(eLayer < LAYER_COUNT); m_eLayer = eLayer; }
	void SetPriority(uint8 uPriority) { m_uPriority = uPriority; SetPriorityCmpValue(); }
	void SetRenderMask(uint32 uMask);
	void SetRenderMaskIncShadow(uint32 uMask);
	uint32 GetRenderMask() const { return m_uRenderMask; }

	Layer	GetLayer() const { return m_eLayer; }
	uint8	GetPriority() const { return m_uPriority; }


	bool operator<(const RenderNode &rhs) const;
	bool operator>(const RenderNode &rhs) const;

	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext) { ASSERT(false); return false; }
	// Should be overloaded, but not necessary for post process effects not ever assigned to a render group
	virtual void RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass) {} // { ASSERT(false); }

	bool GetPostEffect() { return m_bPostEffect; }
	bool HasShadow() { return m_bHasShadow; }

	void SetMaterialCmpVal(const PipelineStateHndl& hndl , const Texture* pTexture0);

	const RenderGroup* GetParent() const { return m_pParent; }
	virtual void VisibilityUpdate(GFXDevice* pDevice, const Vector4f &vTransformOffset) {}

	float GetDepthSortValue() const { return m_pParent->GetSortingDistance(); }
	uint64 GetComparisonValue() const { return m_uComparisonVal; }

protected:
	void SetPostEffect(bool bPostEffect) { m_bPostEffect = bPostEffect; }
	void SetHasShadow(bool bShadow);
	void SetPriorityCmpValue();

private:
	PRIVATIZE_COPY(RenderNode);
	RenderGroup*			m_pParent;
	Layer					m_eLayer;
	uint8					m_uPriority;
	uint32					m_uRenderMask;
	bool					m_bPostEffect;
	bool					m_bHasShadow;
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

