/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/ShadowContext.h"
#include "Engine/Scene/SceneConstantSets.h"
#include <limits>
#include <float.h>
#include "ShadowCascade.h"

//#define CLAMP_TO_TEXEL


#define PCF_BLUR_SIZE 4 // Should get by default with linear sampling

const float FADE_BUFFER = 0.96f;

namespace usg {



static const ShaderConstantDecl g_shadowReadConstDecl[] = 
{
    SHADER_CONSTANT_ELEMENT(ShadowCascade::ShadowReadConstants, mCascadeMtx[0],      CT_MATRIX_44, ShadowCascade::MAX_CASCADES ),
    SHADER_CONSTANT_ELEMENT(ShadowCascade::ShadowReadConstants, mCascadeMtxVInv[0],   CT_MATRIX_44, ShadowCascade::MAX_CASCADES ),

    SHADER_CONSTANT_ELEMENT(ShadowCascade::ShadowReadConstants, vSplitDist,           CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT(ShadowCascade::ShadowReadConstants, vFadeSplitDist,       CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(ShadowCascade::ShadowReadConstants, vInvFadeLength,       CT_VECTOR_4, 1),
    SHADER_CONSTANT_ELEMENT(ShadowCascade::ShadowReadConstants, vBias,                CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT(ShadowCascade::ShadowReadConstants, vSampleRange,         CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(ShadowCascade::ShadowReadConstants, vInvShadowDim,        CT_VECTOR_2, 1),
    SHADER_CONSTANT_END()
};


const ShaderConstantDecl* ShadowCascade::GetDecl()
{
	return g_shadowReadConstDecl;
}

ShadowCascade::ShadowCascade()
{
	m_uGroupWidth = 0;
	m_uGroupHeight = 0;
	m_uCascadeStartIndex = 0;
	m_pRenderTarget = nullptr;

	for (int i = 0; i < MAX_CASCADES; i++)
	{
		m_pSceneContext[i] = nullptr;
	}
}


ShadowCascade::~ShadowCascade()
{
	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		ASSERT(m_pSceneContext[i] == nullptr);
	}
}

void ShadowCascade::Cleanup(GFXDevice* pDevice, Scene* pScene)
{
	m_readConstants.CleanUp(pDevice);
	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		if (m_pSceneContext[i])
		{
			pScene->DeleteShadowContext(m_pSceneContext[i]);
			m_pSceneContext[i] = nullptr;
		}
	}
}

void ShadowCascade::AssignRenderTarget(RenderTarget* pTarget, uint32 uStartIndex)
{
	m_pRenderTarget = pTarget;
	m_uGroupWidth = m_pRenderTarget->GetWidth();
	m_uGroupHeight = m_pRenderTarget->GetHeight();

	const float32 fScale = (float)m_uGroupWidth / 2048.0f;
	const float32 fPartSize[] = { 30.0f, 160.0f, 400.0f, 1000.f };

	for (uint32 i = 0; i < CASCADE_COUNT; i++)
	{
		m_fPartitions0To1[i] = fPartSize[i] * fScale;
	}

	m_fCascadePartitionsMax = fPartSize[CASCADE_COUNT - 1];
}


void ShadowCascade::Init(GFXDevice* pDevice, Scene* pScene, const DirLight* pLight)
{
	m_pLight = pLight;

	// TODO: May want an optimized shadow context
	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		m_pSceneContext[i] = pScene->CreateShadowContext(pDevice);
		m_pSceneContext[i]->Init(&m_cascadeCamera[i]);
		m_pSceneContext[i]->SetActive(true);
	}

	m_readConstants.Init(pDevice, g_shadowReadConstDecl);

}


void ShadowCascade::Update(const Camera& sceneCam)
{
    Vector4f vLookAt = sceneCam.GetPos();
    Vector4f vDirection = m_pLight->GetDirection();
    Vector4f vCameraPos = vLookAt - (vDirection*20.f);
    Vector4f vUp(1.0f, 0.0f, 0.0f, 0.0f);

    //m_shadowCamera.LookAt( vCameraPos.v3(), vLookAt.v3(), vUp.v3());
    m_shadowViewMtx.LookAt( vCameraPos.v3(), vLookAt.v3(), vUp.v3());
	// Init the data
	InitFrame(sceneCam);

    ShadowReadConstants* readData = m_readConstants.Lock<ShadowReadConstants>();

	Matrix4x4 texBias = Matrix4x4::TextureBiasMatrix();
	
    Matrix4x4 mInvView;
    sceneCam.GetViewMatrix().GetInverse(mInvView);
	
    for(uint32 i=0; i<CASCADE_COUNT; i++)
    {
        readData->mCascadeMtx[i] = m_cascadeCamera[i].GetViewMatrix() * m_cascadeCamera[i].GetProjection() * texBias;
        readData->mCascadeMtxVInv[i] = mInvView * m_cascadeCamera[i].GetViewMatrix() * m_cascadeCamera[i].GetProjection() * texBias;
    }

	float fBiasMul = 100.0f;

    readData->vBias.Assign(-0.0000008f*fBiasMul, -0.000004f*fBiasMul, -0.000007f*fBiasMul, -0.000009f*fBiasMul);
	readData->vSampleRange.Assign(3.0f, 2.5f, 2.0f, 0.5f);

	float fadeDistances[MAX_CASCADES];
	float invFadeRange[MAX_CASCADES];
	for (uint32 i = 0; i < MAX_CASCADES; i++)
	{
		fadeDistances[i] = m_fPartitions0To1[i] * FADE_BUFFER;
		invFadeRange[i] = 1.f/(m_fPartitions0To1[i] - fadeDistances[i]);
	}
    readData->vSplitDist.Assign(m_fPartitions0To1[0], m_fPartitions0To1[1], m_fPartitions0To1[2], m_fPartitions0To1[3]);
	readData->vFadeSplitDist.Assign(fadeDistances[0], fadeDistances[1], fadeDistances[2], fadeDistances[3]);
	readData->vInvFadeLength.Assign(invFadeRange[0], invFadeRange[1], invFadeRange[2], invFadeRange[3]);
	readData->vInvShadowDim.Assign(1.f / (float)m_pRenderTarget->GetWidth(), 1.f / (float)m_pRenderTarget->GetHeight());

	m_readConstantsData = *readData;
    m_readConstants.Unlock();
	
}

void ShadowCascade::GPUUpdate(GFXDevice* pDevice)
{
	if (!m_pRenderTarget)
		return;

	m_readConstants.UpdateData(pDevice);
}

void ShadowCascade::ComputeFrustumFromProjection( PointFrustum* pOut, const Matrix4x4* pProjection )
{
    // Frustum corners
    static Vector4f vHomogenous[6];
	vHomogenous[0].Assign( 1.0f,  0.0f, 1.0f, 1.0f );   // right (at far plane)
	vHomogenous[1].Assign( -1.0f, 0.0f, 1.0f, 1.0f );   // left
	vHomogenous[2].Assign( 0.0f,  1.0f, 1.0f, 1.0f );   // top
	vHomogenous[3].Assign( 0.0f, -1.0f, 1.0f, 1.0f );   // bottom

#if Z_RANGE_0_TO_1
    vHomogenous[4].Assign( 0.0f, 0.0f, 0.0f, 1.0f );     // near
#else
	vHomogenous[4].Assign( 0.0f, 0.0f, -1.0f, 1.0f );     // near
#endif
	vHomogenous[5].Assign( 0.0f, 0.0f, 1.0f, 1.0f );      // far

    Matrix4x4 mInverse;
    pProjection->GetInverse(mInverse);

    // Compute the frustum corners in world space.
    Vector4f vPoints[6];
    for(uint32 i = 0; i < 6; i++ )
    {
        // Transform point.
        vPoints[i] = vHomogenous[i] * mInverse;
    }

    pOut->vOrigin = Vector3f( 0.0f, 0.0f, 0.0f );
    pOut->vOrientation = Vector4f( 0.0f, 0.0f, 0.0f, 1.0f );

    const Vector4f vVecOne(1.0f, 1.0f, 1.0f, 1.0f);
    // Compute the slopes.
    vPoints[0] = vPoints[0] * (vVecOne / Vector4f(vPoints[0].z, vPoints[0].z, vPoints[0].z, vPoints[0].z) );
    vPoints[1] = vPoints[1] * (vVecOne / Vector4f(vPoints[1].z, vPoints[1].z, vPoints[1].z, vPoints[1].z) );
    vPoints[2] = vPoints[2] * (vVecOne / Vector4f(vPoints[2].z, vPoints[2].z, vPoints[2].z, vPoints[2].z) );
    vPoints[3] = vPoints[3] * (vVecOne / Vector4f(vPoints[3].z, vPoints[3].z, vPoints[3].z, vPoints[3].z) );

    pOut->fRightSlope	= vPoints[0].x;
    pOut->fLeftSlope	= vPoints[1].x;
    pOut->fTopSlope		= vPoints[2].y;
    pOut->fBottomSlope	= vPoints[3].y;

    // Compute near and far.
	vPoints[4] = vPoints[4] * vVecOne / Vector4f(vPoints[4].w, vPoints[4].w, vPoints[4].w, vPoints[4].w);
    vPoints[5] = vPoints[5] * vVecOne / Vector4f(vPoints[5].w, vPoints[5].w, vPoints[5].w, vPoints[5].w);

    pOut->fNear = vPoints[4].z;
    pOut->fFar = vPoints[5].z;
}

void ShadowCascade::CreateFrustumPointsFromInterval( float fIntervalBegin, float fIntervalEnd,const Matrix4x4& mViewProj, Vector4f* pvPointsWorld ) 
{
	PointFrustum viewFrust;
    ComputeFrustumFromProjection(&viewFrust, &mViewProj);

    viewFrust.fNear = fIntervalBegin;
    viewFrust.fFar = fIntervalEnd;


    Vector4f vRightTop(viewFrust.fRightSlope,viewFrust.fTopSlope,1.0f,1.0f);
    Vector4f vLeftBottom(viewFrust.fLeftSlope,viewFrust.fBottomSlope,1.0f,1.0f);
    Vector4f vNear(viewFrust.fNear,viewFrust.fNear,viewFrust.fNear,1.0f);
    Vector4f vFar(viewFrust.fFar,viewFrust.fFar,viewFrust.fFar,1.0f);
    Vector4f vRightTopNear		= vRightTop * vNear;
    Vector4f vRightTopFar		= vRightTop * vFar;
    Vector4f vLeftBottomNear	= vLeftBottom * vNear;
    Vector4f vLeftBottomFar		= vLeftBottom * vFar;

    pvPointsWorld[0] = vRightTopNear;
    pvPointsWorld[1] = Vector4f(vLeftBottomNear.x, vRightTopNear.y, vRightTopNear.z, vRightTopNear.w );
    pvPointsWorld[2] = vLeftBottomNear;
    pvPointsWorld[3] = Vector4f( vRightTopNear.x, vLeftBottomNear.y, vRightTopNear.z, vRightTopNear.w );

    pvPointsWorld[4] = vRightTopFar;
    pvPointsWorld[5] = Vector4f( vLeftBottomFar.x, vRightTopFar.y, vRightTopFar.z, vRightTopFar.w);
    pvPointsWorld[6] = vLeftBottomFar;
    pvPointsWorld[7] = Vector4f( vRightTopFar.x, vLeftBottomFar.y, vRightTopFar.z, vRightTopFar.w );

}


void ShadowCascade::InitFrame(const Camera& sceneCam)
{
    const Matrix4x4& mCameraProj = sceneCam.GetProjection();
	const Matrix4x4& mCameraView = sceneCam.GetViewMatrix();

    const Matrix4x4& mLightView = m_shadowViewMtx;//m_shadowCamera.GetViewMatrix();
	Matrix4x4 mInvCameraView;
	mCameraView.GetInverse(mInvCameraView);

	AABB worldBounds = m_pSceneContext[0]->GetScene()->GetWorldBounds();

    Vector3f vSceneCenter = worldBounds.GetPos();
    Vector3f vSceneExtents = worldBounds.GetRadii();

    AABB::BoxCorners corners;
	worldBounds.GetCorners( corners );

    // Transform the world bounds into light space
	Vector4f vSceneAABBLightSpace[8];
    for(uint32 uCorner=0; uCorner<8; uCorner++) 
    {
        vSceneAABBLightSpace[uCorner] = Vector4f( corners.verts[uCorner], 1.0f) *  mLightView;
    }

    
    float fFrustumIntervalBegin, fFrustumIntervalEnd;
    Vector4f vLightCameraOrthoMin;  // light space frustrum aabb 
    Vector4f vLightCameraOrthoMax;
    float fCameraNearFarRange = sceneCam.GetFar() - sceneCam.GetNear();

	Vector4f vWorldUnitsPerTexel = Vector4f(0.0f, 0.0f, 0.0f, 0.0f); 

    // Calculate the orthographic projection matrix for each cascade
    for( uint32 uCascade=0; uCascade < CASCADE_COUNT; ++uCascade ) 
    {
		if (uCascade == 0)
		{
			fFrustumIntervalBegin = 0.0f;
		}
		else
		{
			fFrustumIntervalBegin = m_fPartitions0To1[uCascade - 1] * FADE_BUFFER;
		}


        // Scale the intervals between 0 and 1. They are now percentages that we can scale with.
        fFrustumIntervalEnd		= m_fPartitions0To1[ uCascade ] / m_fCascadePartitionsMax;        
        fFrustumIntervalBegin 	= fFrustumIntervalBegin / m_fCascadePartitionsMax;
        fFrustumIntervalBegin	= fFrustumIntervalBegin * fCameraNearFarRange;
        fFrustumIntervalEnd		= fFrustumIntervalEnd * fCameraNearFarRange;
        Vector4f vFrustumPoints[8];

        // Get the 8 points for the frustum of this particular cascade interval
        CreateFrustumPointsFromInterval( fFrustumIntervalBegin, fFrustumIntervalEnd, 
            mCameraProj, vFrustumPoints );

        vLightCameraOrthoMin.Assign(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
        vLightCameraOrthoMax.Assign(FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN);

        Vector4f vTmpCornerPoint;
        // This next section of code calculates the min and max values for the orthographic projection.
        for( int icpIndex=0; icpIndex < 8; ++icpIndex ) 
        {
            // Transform the frustum from camera view space to world space.
            vFrustumPoints[icpIndex] = vFrustumPoints[icpIndex] * mInvCameraView;
            // Transform the point from world space to Light Camera Space.
            vTmpCornerPoint = vFrustumPoints[icpIndex] * mLightView;
            // Find the closest point.
            vLightCameraOrthoMin = Vec4fMin( vTmpCornerPoint, vLightCameraOrthoMin );
            vLightCameraOrthoMax = Vec4fMax( vTmpCornerPoint, vLightCameraOrthoMax );
        }

        
        // Adjust to the blur size
        float fScaleDuetoBlureAMT = ( (float)( PCF_BLUR_SIZE * 2 + 1 ) 
            /(float)m_uGroupWidth );
        Vector4f vScaleDuetoBlureAMT(fScaleDuetoBlureAMT, fScaleDuetoBlureAMT, 0.0f, 0.0f);

        
        float fNormalizeByBufferSize = ( 1.0f / (float)m_uGroupWidth );
        Vector4f vNormalizeByBufferSize( fNormalizeByBufferSize, fNormalizeByBufferSize, 0.0f, 0.0f );

        Vector4f vBoarderOffset = vLightCameraOrthoMax - vLightCameraOrthoMin;
      //  vBoarderOffset *= 0.5f;
        vBoarderOffset *= vScaleDuetoBlureAMT;
        vLightCameraOrthoMax += vBoarderOffset;
        vLightCameraOrthoMin -= vBoarderOffset;
       
        // Because we're fitting tighly to the cascades, the shimmering shadow edges will still be present when the 
        // camera rotates.  However, when zooming in or strafing the shadow edge will not shimmer.
        vWorldUnitsPerTexel = vLightCameraOrthoMax - vLightCameraOrthoMin;
        vWorldUnitsPerTexel *= vNormalizeByBufferSize;

#ifdef CLAMP_TO_TEXEL
        // We snape the camera to 1 pixel increments so that moving the camera does not cause the shadows to jitter.
        // This is a matter of integer dividing by the world space size of a texel
        vLightCameraOrthoMin /= vWorldUnitsPerTexel;
        vLightCameraOrthoMin = vLightCameraOrthoMin.GetFloor();
        vLightCameraOrthoMin *= vWorldUnitsPerTexel;
            
        vLightCameraOrthoMax /= vWorldUnitsPerTexel;
        vLightCameraOrthoMax = vLightCameraOrthoMax.GetFloor();
        vLightCameraOrthoMax *= vWorldUnitsPerTexel;
#endif
 
     
		float fNearPlane = 0.0f;
		float fFarPlane = 10000.0f;

		// FIXME: Need better bounds calculation
		Vector4f vLightSpaceSceneAABBMin(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
		Vector4f vLightSpaceSceneAABBMax(FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN);
	 
		// Get the min and max values for the AABB in light space
		for(uint32 uCorner=0; uCorner< 8; ++uCorner)
		{
			vLightSpaceSceneAABBMin = Vec4fMin( vSceneAABBLightSpace[uCorner], vLightSpaceSceneAABBMin );
			vLightSpaceSceneAABBMax = Vec4fMax( vSceneAABBLightSpace[uCorner], vLightSpaceSceneAABBMax );
		}

		// Use the min and max Z values as the near and far places
		fNearPlane = vLightSpaceSceneAABBMin.z;
		fFarPlane = vLightSpaceSceneAABBMax.z;


        // Create the orthographic projection for this cascade.
        m_matShadowProj[ uCascade ].Orthographic( vLightCameraOrthoMin.x, vLightCameraOrthoMax.x, vLightCameraOrthoMin.y, vLightCameraOrthoMax.y, fNearPlane, fFarPlane );

        m_fCascadePartitionsFrustum[ uCascade ] = fFrustumIntervalEnd;

        m_cascadeCamera[uCascade].SetUp( m_shadowViewMtx, m_matShadowProj[uCascade] );
    }
 }

void ShadowCascade::CreateShadowTex(GFXContext* pContext)
{
	if (!m_pRenderTarget)
		return;

    pContext->BeginGPUTag("Shadow");


	for (uint32 i = 0; i < CASCADE_COUNT; ++i)
    {
        pContext->SetRenderTargetLayer(m_pRenderTarget, m_uCascadeStartIndex + i);
		DrawSceneFromLight(pContext, m_pSceneContext[i]);
        pContext->SetRenderTarget(NULL);
	}

    pContext->EndGPUTag();
}


void ShadowCascade::Finished(GFXContext* pContext)
{

}


void ShadowCascade::DrawSceneFromLight(GFXContext* pGFXCtxt, ShadowContext* pShadowCtxt)
{
	pShadowCtxt->DrawScene(pGFXCtxt);
    
}


}
