/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A cascade shadow system, the scene is split into various layers
*****************************************************************************/
#ifndef _USG_POSTFX_SHADOWS_SHADOW_CASCADE_H_
#define _USG_POSTFX_SHADOWS_SHADOW_CASCADE_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "Engine/Scene/Camera/StandardCamera.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Graphics/Shadows/DirectionalShadow.h"

namespace usg
{

class ShadowContext;
class GFXContext;
class DirLight;

#define RENDER_TO_LAYER 1

class ShadowCascade : public DirectionalShadow
{
public:
	ShadowCascade();
	virtual ~ShadowCascade();

	virtual void Init(GFXDevice* pDevice, Scene* pScene, const DirLight* pLight, uint32 uGroupWidth, uint32 uGroupHeight);
	void Cleanup(GFXDevice* pDevice, Scene* pScene);
	virtual void Update(const Camera &sceneCam);
	void GPUUpdate(GFXDevice* pDevice);
	virtual void CreateShadowTex(GFXContext* pContext);
	virtual void Finished(GFXContext* pContext);
	void PrepareRender(GFXContext* pContext) const override;

	const ConstantSet* GetShadowReadConstants() { return &m_readConstants; }
	ShadowContext* GetContext(uint32 uContext) { return m_pSceneContext[uContext]; }

	enum
	{
		CASCADE_COUNT = 4,	// MAX is 4
		MAX_CASCADES = 4
	};

private:

	struct PointFrustum
	{
		Vector3f vOrigin;
	    Vector4f vOrientation;

	    float fRightSlope;
	    float fLeftSlope;
	    float fTopSlope;
	    float fBottomSlope;
	    float fNear, fFar;
	};

	void ComputeFrustumFromProjection(PointFrustum* pOut, const Matrix4x4* pProjection );
	void CreateFrustumPointsFromInterval(float fIntervalBegin, float fIntervalEnd,const Matrix4x4& mViewProj, Vector4f* pvPointsWorld);
	void DrawSceneFromLight(GFXContext* pGFXCtxt, ShadowContext* pShadowCtxt);
	void InitFrame(const Camera& sceneCam);

	uint32					m_uGroupWidth;
	uint32					m_uGroupHeight;

	Matrix4x4				m_lightView;
	Matrix4x4				m_lightProj;

	float					m_fPartitions0To1[MAX_CASCADES];
	float					m_fCascadePartitionsFrustum[MAX_CASCADES];
	float					m_fCascadePartitionsMax;

	// One frustum for each cascade
	PointFrustum			m_frustum[MAX_CASCADES];
	Matrix4x4				m_matShadowProj[MAX_CASCADES];

	DescriptorSet			m_readDescriptor;

	const DirLight*			m_pLight;
	ConstantSet				m_readConstants;

	Matrix4x4				m_shadowViewMtx;
	StandardCamera			m_cascadeCamera[MAX_CASCADES];
	ShadowContext*			m_pSceneContext[MAX_CASCADES];

#if !RENDER_TO_LAYER
	DepthStencilBuffer		m_depthBuffer;
	RenderTarget			m_depthTarget;
	ColorBuffer				m_cascadeBuffer;
#else
	DepthStencilBuffer		m_cascadeBuffer;
#endif
	
	RenderTarget			m_cascadeTarget;
};

}


#endif
