/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Maths/Sphere.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/OmniShadowContext.h"
#include "Engine/Graphics/Device/GFXContext.h"

namespace usg {

struct GlobalOmniShadowConstants
{
	Matrix4x4 	mMatrices[6];
	Vector4f	vLightPos;
	float 	 	fLightFarDist;
};

static const ShaderConstantDecl g_globalOmniShadowCBDecl[] =
{
	SHADER_CONSTANT_ELEMENT(GlobalOmniShadowConstants, mMatrices,			CT_MATRIX_44, 6),
	SHADER_CONSTANT_ELEMENT(GlobalOmniShadowConstants, vLightPos,			CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(GlobalOmniShadowConstants, fLightFarDist,		CT_FLOAT, 1),
	SHADER_CONSTANT_END()
};


OmniShadowContext::OmniShadowContext():
SceneContext()
{
}

OmniShadowContext::~OmniShadowContext()
{

}


void OmniShadowContext::InitDeviceData(GFXDevice* pDevice)
{
	m_globalConstants.Init(pDevice, g_globalOmniShadowCBDecl);
	m_descriptorSet.Init(pDevice, pDevice->GetDescriptorSetLayout(SceneConsts::g_omniShadowGlobalDescriptorDecl));
	m_descriptorSet.SetConstantSet(0, &m_globalConstants);
	m_descriptorSet.UpdateDescriptors(pDevice);

	SetDeviceDataLoaded();
}

void OmniShadowContext::Cleanup(GFXDevice* pDevice)
{
	m_globalConstants.Cleanup(pDevice);
	m_descriptorSet.Cleanup(pDevice);
}

void OmniShadowContext::Init(const Sphere* sphere)
{
	SetRenderMask(RenderMask::RENDER_MASK_SHADOW_CAST);
	m_pSphere = sphere;
	m_searchObject.Init(GetScene(), this, *m_pSphere, RenderMask::RENDER_MASK_SHADOW_CAST);
}

void OmniShadowContext::ClearLists()
{
	m_drawList.clear();
	Inherited::ClearLists();
}

void OmniShadowContext::Update(GFXDevice* pDevice)
{
	const Camera* pCamera = GetCamera();

	GlobalOmniShadowConstants* globalData = m_globalConstants.Lock<GlobalOmniShadowConstants>();

	Matrix4x4 mProj;
	mProj.Perspective(usg::Math::pi_over_2, 1.0f, 0.1f, m_pSphere->GetRadius());
	Vector4f vPos = Vector4f(m_pSphere->GetPos(), 1.0f);

	globalData->vLightPos = vPos;
	globalData->fLightFarDist = m_pSphere->GetRadius();

	Matrix4x4 mFaceMat = Matrix4x4::Identity();
	// +x
	mFaceMat.CameraMatrix(-V4F_Z_AXIS, V4F_Y_AXIS, V4F_X_AXIS, vPos);
	globalData->mMatrices[0] = mFaceMat * mProj;
	// -x
	mFaceMat.CameraMatrix(V4F_Z_AXIS, V4F_Y_AXIS, -V4F_X_AXIS, vPos);
	globalData->mMatrices[1] = mFaceMat * mProj;

	// Reversing these as we are flipping co-ordinates for opengl
#ifdef OGL_UVS
	// +y
	mFaceMat.CameraMatrix(V4F_X_AXIS, -V4F_Z_AXIS, V4F_Y_AXIS, vPos);
	globalData->mMatrices[3] = mFaceMat * mProj;
	// -y
	mFaceMat.CameraMatrix(V4F_X_AXIS, V4F_Z_AXIS, -V4F_Y_AXIS, vPos);
	globalData->mMatrices[2] = mFaceMat * mProj;
#else
	// +y
	mFaceMat.CameraMatrix(V4F_X_AXIS, -V4F_Z_AXIS, V4F_Y_AXIS, vPos);
	globalData->mMatrices[2] = mFaceMat * mProj;
	// -y
	mFaceMat.CameraMatrix(V4F_X_AXIS, V4F_Z_AXIS, -V4F_Y_AXIS, vPos);
	globalData->mMatrices[3] = mFaceMat * mProj;
#endif

	// +z
	mFaceMat.CameraMatrix(V4F_X_AXIS, V4F_Y_AXIS, V4F_Z_AXIS, vPos);
	globalData->mMatrices[4] = mFaceMat * mProj;
	// -z
	mFaceMat.CameraMatrix(-V4F_X_AXIS, V4F_Y_AXIS, -V4F_Z_AXIS, vPos);
	globalData->mMatrices[5] = mFaceMat * mProj;

	m_globalConstants.Unlock();
	m_globalConstants.UpdateData(pDevice);
	m_descriptorSet.UpdateDescriptors(pDevice);

	for (RenderGroup* pGroup : GetVisibleGroups())
	{
		uint32 uNodeCount = pGroup->GetLODEntryCount(0);
		for(uint32 i=0; i < uNodeCount; i++ )
		{
			RenderNode* pNode = pGroup->GetLODRenderNode(0, i);
			if( (pNode->GetRenderMask() & RenderMask::RENDER_MASK_SHADOW_CAST)!=0 )
			{
				m_drawList.push_back(pNode);
			}
		}
	}
}


void OmniShadowContext::DrawScene(GFXContext* pContext)
{
	pContext->SetDescriptorSet( &m_descriptorSet, 0 );
	RenderNode::RenderContext renderContext;
	renderContext.eRenderPass = RenderNode::RENDER_PASS_DEPTH_OMNI;

	for(RenderNode* node : m_drawList)
	{
		node->Draw(pContext, renderContext);
	}
}

}
