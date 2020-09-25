/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#if 1
#include "Engine/Maths/Vector2f.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Lights/LightMgr.h"
#include "Engine/Graphics/Lights/PointLight.h"
#include "Engine/Graphics/Lights/SpotLight.h"
#include "Engine/Graphics/Lights/ProjectionLight.h"
#include "Engine/Scene/LightingContext.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Shadows/ShadowCascade.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Resource/ResourceMgr.h"
#include FRAGMENT_HEADER(Engine/PostFX/, DeferredShading.h)


namespace usg {

static const DescriptorDeclaration g_descriptorGBuffer[] =
{
	DESCRIPTOR_ELEMENT(0,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(1,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(2,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(3,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(4,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};



DeferredShading::DeferredShading():
PostEffect()
{
	SetRenderMask(RENDER_MASK_LIGHTING);
	SetLayer(LAYER_DEFERRED_SHADING);
	SetPriority(127);	// Comes last
	m_pSys = NULL;
	m_pDestTarget = NULL;
}

DeferredShading::~DeferredShading()
{

}


void DeferredShading::Init(GFXDevice* pDevice, ResourceMgr* pRes, PostFXSys* pSys, RenderTarget* pDst)
{
	m_pSys = pSys;

	U8String name;

	// Directional lighting, test against the terrain stencil value of 2
	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;

	RenderPassHndl renderPassHndl = pDst->GetRenderPass();

	pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineDecl.layout.descriptorSets[1] = pDevice->GetDescriptorSetLayout(g_descriptorGBuffer);
	pipelineDecl.layout.uDescriptorSetCount = 2;

	// Standard depth stencil
	DepthStencilStateDecl& depthDecl = pipelineDecl.depthState;
	depthDecl.bDepthWrite		= false;
	depthDecl.bDepthEnable		= false;
	depthDecl.eDepthFunc		= DEPTH_TEST_ALWAYS;
	depthDecl.bStencilEnable	= true;
	depthDecl.SetOperation(STENCIL_OP_KEEP, STENCIL_OP_KEEP, STENCIL_OP_KEEP);
	depthDecl.eStencilTest		= STENCIL_TEST_ALWAYS;
	depthDecl.SetMask(STENCIL_MASK_GEOMETRY, STENCIL_MASK_EFFECT, STENCIL_GEOMETRY);

	// Alpha disabled
	AlphaStateDecl& alphaDecl = pipelineDecl.alphaState;
	alphaDecl.bBlendEnable = false;
	alphaDecl.SetColor0Only();

	// Front face
	RasterizerStateDecl& rasDecl = pipelineDecl.rasterizerState;
	rasDecl.eCullFace = CULL_FACE_NONE;

	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.DirBase");
	m_baseDirPass = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);

	
	alphaDecl.bBlendEnable = true;
	alphaDecl.blendEq = BLEND_EQUATION_ADD;
	alphaDecl.srcBlend = BLEND_FUNC_ONE;
	alphaDecl.dstBlend = BLEND_FUNC_ONE;
	for (uint32 i = 0; i < MAX_EXTRA_DIR_LIGHTS; i++)
	{
		name.ParseString("Deferred.DirExtraShadow.%d", i + 1);
		pipelineDecl.pEffect = pRes->GetEffect(pDevice, name.CStr());
		m_additionalShadowPass[i] = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	}
	alphaDecl.bBlendEnable = false;
	
	// Color disabled alpha
	alphaDecl.SetDepthOnly();

	// Back face
	rasDecl.eCullFace = CULL_FACE_NONE;

	// Clear depth stencil	
	depthDecl.bDepthWrite = false;
	depthDecl.bDepthEnable = true;
	depthDecl.eDepthFunc = DEPTH_TEST_GEQUAL;
	depthDecl.bStencilEnable = true;
	depthDecl.eStencilTest = STENCIL_TEST_EQUAL;
	// If geometry is obscuring the back face then write 0x1 to the stencil buffer
	depthDecl.SetOperation(STENCIL_OP_REPLACE, STENCIL_OP_KEEP, STENCIL_OP_ZERO, false, true);
	depthDecl.SetMask(STENCIL_MASK_GEOMETRY, 0x1, 0x1 | STENCIL_GEOMETRY, false, true);
	// If geometry is obscuring the front face then we don't want to render the light, write 0x2 to indicate this
	depthDecl.SetOperation(STENCIL_OP_REPLACE, STENCIL_OP_KEEP, STENCIL_OP_ZERO, true, false);
	depthDecl.SetMask(STENCIL_MASK_GEOMETRY, 0x2, 0x2 | STENCIL_GEOMETRY, true, false);


	// Light volume back face
	pipelineDecl.layout.uDescriptorSetCount = 3;
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(ProjectionLight::GetDescriptorDecl());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.ProjectionPos");
	m_projShaders.pStencilWriteEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(PointLight::GetDescriptorDecl());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.PointPos");
	m_sphereShaders.pStencilWriteEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(SpotLight::GetDescriptorDecl());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.SpotPos");;
	m_spotShaders.pStencilWriteEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);

	// Lighting when not intersecting far plane
	depthDecl.bDepthWrite		= false;
	depthDecl.bDepthEnable		= true;
	depthDecl.eDepthFunc		= DEPTH_TEST_ALWAYS;
	depthDecl.bStencilEnable	= true;
	depthDecl.SetOperation(STENCIL_OP_ZERO, STENCIL_OP_ZERO, STENCIL_OP_ZERO);
	depthDecl.eStencilTest		= STENCIL_TEST_EQUAL;
	// Only render the light where geometry has been written, back 
	depthDecl.SetMask(STENCIL_MASK_GEOMETRY|STENCIL_MASK_EFFECT, 0x03, 0x01 | STENCIL_GEOMETRY);

	// Back face

	rasDecl.eCullFace = CULL_FACE_FRONT;

	// Additive alpha
	alphaDecl.SetColor0Only();
	alphaDecl.bBlendEnable = true;
	alphaDecl.blendEq = BLEND_EQUATION_ADD;
	alphaDecl.dstBlend = BLEND_FUNC_ONE;
	alphaDecl.srcBlend = BLEND_FUNC_ONE;


	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(PointLight::GetDescriptorDecl());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Point");
	m_sphereShaders.pLightingEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.PointNoSpec");
	m_sphereShaders.pLightingNoSpecEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(PointLight::GetDescriptorDeclShadow());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Point.shadow");
	m_sphereShaders.pLightingShadowEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);

	// Fixme no spec versions
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(SpotLight::GetDescriptorDecl());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Spot");
	m_spotShaders.pLightingEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	m_spotShaders.pLightingNoSpecEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(SpotLight::GetDescriptorDeclShadow());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Spot.shadow");
	m_spotShaders.pLightingShadowEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);


	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(ProjectionLight::GetDescriptorDecl());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Projection");
	m_projShaders.pLightingEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Projection");
	m_projShaders.pLightingNoSpecEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(ProjectionLight::GetDescriptorDeclShadow());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Projection.shadow");
	m_projShaders.pLightingShadowEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);

	// Front face
	rasDecl.eCullFace = CULL_FACE_BACK;

	// Far plane intersecting lighting
	depthDecl.bDepthWrite		= false;
	depthDecl.bDepthEnable		= true;
	depthDecl.eDepthFunc		= DEPTH_TEST_LESS;
	depthDecl.bStencilEnable	= true;
	depthDecl.SetOperation(STENCIL_OP_KEEP, STENCIL_OP_KEEP, STENCIL_OP_KEEP);
	depthDecl.eStencilTest		= STENCIL_TEST_EQUAL;
	depthDecl.SetMask(STENCIL_MASK_GEOMETRY, STENCIL_MASK_EFFECT, STENCIL_GEOMETRY);

	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(PointLight::GetDescriptorDecl());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Point");
	m_sphereShaders.pLightingFarPlaneEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.PointNoSpec");
	m_sphereShaders.pLightingFarPlaneNoSpecEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(PointLight::GetDescriptorDeclShadow());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Point.shadow");
	m_sphereShaders.pLightingFarPlaneShadowEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);

	// Fixme no spec versions
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(SpotLight::GetDescriptorDecl());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Spot");
	m_spotShaders.pLightingFarPlaneEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Spot");
	m_spotShaders.pLightingFarPlaneNoSpecEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(SpotLight::GetDescriptorDeclShadow());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Spot.shadow");
	m_spotShaders.pLightingFarPlaneShadowEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);

	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(ProjectionLight::GetDescriptorDecl());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Projection");
	m_projShaders.pLightingFarPlaneEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Projection");
	m_projShaders.pLightingFarPlaneNoSpecEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);
	pipelineDecl.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(ProjectionLight::GetDescriptorDeclShadow());
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "Deferred.Projection.shadow");
	m_projShaders.pLightingFarPlaneShadowEffect = pDevice->GetPipelineState(renderPassHndl, pipelineDecl);


	SamplerDecl samplerDecl(SF_POINT, SC_CLAMP);
	// TODO: Move to the device!!
	m_samplerHndl = pDevice->GetSampler(samplerDecl);

	samplerDecl.SetFilter(SF_LINEAR);

	m_linSamplerHndl = pDevice->GetSampler(samplerDecl);


	DescriptorSetLayoutHndl texReadDesc = pDevice->GetDescriptorSetLayout(g_descriptorGBuffer);
	m_readDescriptors.Init(pDevice, texReadDesc);

	MakeSphere(pDevice);
	MakeCone(pDevice);
	MakeFrustum(pDevice);
}

void DeferredShading::Cleanup(GFXDevice* pDevice)
{
	m_readDescriptors.Cleanup(pDevice);
	m_sphereVB.Cleanup(pDevice);
	m_sphereIB.Cleanup(pDevice);
	m_coneVB.Cleanup(pDevice);
	m_coneIB.Cleanup(pDevice);
	m_frustumMesh.Cleanup(pDevice);
	m_frustumIB.Cleanup(pDevice);
}


void DeferredShading::SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget)
{
	m_readDescriptors.SetImageSamplerPairAtBinding(0, pTarget->GetColorTexture(DT_LINEAR_DEPTH),	m_samplerHndl);
	m_readDescriptors.SetImageSamplerPairAtBinding(1, pTarget->GetColorTexture(DT_NORMAL),			m_samplerHndl);
	m_readDescriptors.SetImageSamplerPairAtBinding(2, pTarget->GetColorTexture(DT_DIFFUSE_COL),		m_samplerHndl);
	m_readDescriptors.SetImageSamplerPairAtBinding(3, pTarget->GetColorTexture(DT_EMISSIVE),		m_samplerHndl);
	m_readDescriptors.SetImageSamplerPairAtBinding(4, pTarget->GetColorTexture(DT_SPEC_COL),		m_samplerHndl);
	m_readDescriptors.UpdateDescriptors(pDevice);
}

void DeferredShading::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	// The internal data has changed
	m_readDescriptors.UpdateDescriptors(pDevice);
}


void DeferredShading::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
{ 
	if (m_pDestTarget != pDst)
	{
		RenderPassHndl renderPassHndl = pDst->GetRenderPass();
		// This is obviously not ideal, but it shouldn't normally happen. You'd have to be turning off bloom at run time.
		// If it does happen we can cache all the combinations we see
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_baseDirPass);

		for (uint32 i = 0; i < MAX_EXTRA_DIR_LIGHTS; i++)
		{
			pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_additionalShadowPass[i]);
		}

		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_projShaders.pStencilWriteEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_sphereShaders.pStencilWriteEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_spotShaders.pStencilWriteEffect);
		
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_projShaders.pLightingFarPlaneEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_sphereShaders.pLightingFarPlaneEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_spotShaders.pLightingFarPlaneEffect);

		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_projShaders.pLightingEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_sphereShaders.pLightingEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_spotShaders.pLightingEffect);

		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_projShaders.pLightingFarPlaneNoSpecEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_sphereShaders.pLightingFarPlaneNoSpecEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_spotShaders.pLightingFarPlaneNoSpecEffect);

		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_projShaders.pLightingFarPlaneShadowEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_sphereShaders.pLightingFarPlaneShadowEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_spotShaders.pLightingFarPlaneShadowEffect);

		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_projShaders.pLightingNoSpecEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_sphereShaders.pLightingNoSpecEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_spotShaders.pLightingNoSpecEffect);

		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_projShaders.pLightingShadowEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_sphereShaders.pLightingShadowEffect);
		pDevice->ChangePipelineStateRenderPass(renderPassHndl, m_spotShaders.pLightingShadowEffect);

		m_pDestTarget = pDst;
	}
}

bool DeferredShading::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	ViewContext* pSceneCtxt = m_pSys->GetActiveViewContext();

	uint32 uLightCount = 0;
	const List<DirLight>& dirLights = pSceneCtxt->GetLightingContext().GetActiveDirLights();

	uLightCount = dirLights.GetSize();

	pContext->SetRenderTarget(m_pDestTarget);
	// Set the textures for the G buffer to be read from
	pContext->SetDescriptorSet(&m_readDescriptors, 1);

	pContext->BeginGPUTag("DirLights", Color::Green);
	//if(uLightCount > 0)
	{
		// FIXME: We need to know if we need a shadow or not!!!
		PipelineStateHndl& effect = m_baseDirPass;

		// We're not using the standard material system for directional lights as we need to bind multiple buffers
		pContext->SetPipelineState(effect);

		// Don't need to do this as it should already be bound with the globals
		//pSceneCtxt->GetLightingContext().BindDirectionalLights(pContext);


		m_pSys->DrawFullScreenQuad(pContext);
	}

	// The first shadowed directional light will be drawn by the default pass, if we have additional shadowed dir lights produce them here
	const DirLight* pFirstShadowedLight = nullptr;
	{
		uint32 uLightIndex = 0;
		uint32 uShadowIndex = 0;
		for (List<DirLight>::Iterator it = dirLights.Begin(); !it.IsEnd(); ++it)
		{
			if ((*it)->GetShadowEnabled())
			{
				if (uShadowIndex > 0)
				{
					PipelineStateHndl& effect = m_additionalShadowPass[uLightIndex-1];
					//(*it)->GetCascade()->PrepareRender(pContext);
					pContext->SetPipelineState(effect);
					m_pSys->DrawFullScreenQuad(pContext);
				}
				else
				{
					pFirstShadowedLight = (*it);
				}
				uShadowIndex++;
			}
			uLightIndex++;
			if (uLightIndex > MAX_EXTRA_DIR_LIGHTS)
			{
				// We have hardcoded a max number for now
				break;
			}
		}
	}

	pContext->EndGPUTag();
	const Frustum& frustum = m_pSys->GetActiveViewContext()->GetCamera()->GetFrustum();
	const Plane& farPlane = frustum.GetPlane( Frustum::PLANE_FAR );
	MeshData 		mesh;
 
 	// FIXME: All of this can be merged by moving the geometry and plane classification into the lights themselves
 	// Then we can iterate through all volume lights at once
	pContext->BeginGPUTag("PointLights", Color::Green);
	mesh.pVB = &m_sphereVB;
	mesh.pIB = &m_sphereIB;
	
	const List<PointLight>& pointLights = pSceneCtxt->GetLightingContext().GetPointLightsInView();
	for(List<PointLight>::Iterator it = pointLights.Begin(); !it.IsEnd(); ++it)
	{
		PlaneClass eClass = farPlane.GetSpherePlaneClass( (*it)->GetColSphere());
		mesh.pDescriptorSet = (*it)->GetDescriptorSet(false);
		mesh.pShadowDescriptorSet = (*it)->GetDescriptorSet(true);
		
		switch( eClass )
		{
			// FIXME: Spec shader being picked on r, used to be a single values
			case PC_IN_FRONT:
				DrawLightVolume(pContext, mesh, m_sphereShaders, (*it)->GetSpecular().r() > 0.0f, (*it)->GetShadowEnabled());
				break;
			case PC_ON_PLANE:
				DrawLightVolumeFarPlane(pContext,mesh, m_sphereShaders, (*it)->GetSpecular().r() > 0.0f, (*it)->GetShadowEnabled());
				break;
			default:
				ASSERT(false);
		}
	}
	pContext->EndGPUTag();

	pContext->BeginGPUTag("SpotLights", Color::Green);
	mesh.pVB = &m_coneVB;
	mesh.pIB = &m_coneIB;

	const List<SpotLight>& spotLights = pSceneCtxt->GetLightingContext().GetSpotLightsInView();
	for (List<SpotLight>::Iterator it = spotLights.Begin(); !it.IsEnd(); ++it)
	{
		PlaneClass eClass = farPlane.GetSpherePlaneClass( (*it)->GetColSphere());
		mesh.pDescriptorSet = (*it)->GetDescriptorSet(false);
		mesh.pShadowDescriptorSet = (*it)->GetDescriptorSet(true);

		switch( eClass )
		{
			case PC_IN_FRONT:
				DrawLightVolume(pContext, mesh, m_spotShaders, true, (*it)->GetShadowEnabled());
				break;
			case PC_ON_PLANE:
				DrawLightVolumeFarPlane(pContext, mesh, m_spotShaders, true, (*it)->GetShadowEnabled());
				break;
			default:
				ASSERT(false);
		}
	}
	pContext->EndGPUTag();

	DrawProjectionLights(pContext);

	// We may have overwritten the shadow texture and constants, so set the primary shadowed directional light up for use during forward shading
	if (pFirstShadowedLight)
	{
		// Reset the first shadowed lights constants for the subsequent forward rendering
//		pFirstShadowedLight->GetCascade()->PrepareRender(pContext);
	}


	return true;
}


void DeferredShading::DrawProjectionLights(GFXContext* pContext)
{
	ViewContext* pSceneCtxt = m_pSys->GetActiveViewContext();
	pContext->BeginGPUTag("ProjLights", Color::Green);
	MeshData 		mesh;
	mesh.pVB = &m_frustumMesh;
	mesh.pIB = &m_frustumIB;


	const List<ProjectionLight>& projLights = pSceneCtxt->GetLightingContext().GetProjLightsInView();
	for (List<ProjectionLight>::Iterator it = projLights.Begin(); !it.IsEnd(); ++it)
	{
		ProjectionLight* pLight = (*it);
		mesh.pDescriptorSet = (*it)->GetDescriptorSet(false);
		mesh.pShadowDescriptorSet = (*it)->GetDescriptorSet(true);

		// Tex reads are at 1
		pContext->SetDescriptorSet(mesh.pDescriptorSet, 2);


		const Frustum& frustum = m_pSys->GetActiveViewContext()->GetCamera()->GetFrustum();
		const Plane& farPlane = frustum.GetPlane(Frustum::PLANE_FAR);
		bool bOnFarPlane = false;
		for(uint32 uCorner = 0; uCorner < 8; uCorner++)
		{
			const Vector4f vPoint = pLight->GetCorner(uCorner);
			if (farPlane.GetPointPlaneClass(vPoint.v3()) != PC_IN_FRONT)
			{
				bOnFarPlane = true;
				break;
			}
		}

		if(!bOnFarPlane)
		{
			DrawLightVolume(pContext, mesh, m_projShaders, true, (*it)->GetShadowEnabled());
		}
		else
		{
			DrawLightVolumeFarPlane(pContext, mesh, m_projShaders, true, (*it)->GetShadowEnabled());
		}
	}
	pContext->EndGPUTag();
}


void DeferredShading::DrawLightVolume(GFXContext* pContext, const MeshData& mesh, const VolumeShader& shaders, bool bSpecular, bool bShadow)
{
	// Fill the stencil buffer for the back faces
	pContext->SetPipelineState(shaders.pStencilWriteEffect);
	pContext->SetDescriptorSet(mesh.pDescriptorSet, 2);
	DrawMesh(pContext, mesh);
	

	// Perform the lighting pass
	if (bShadow)
	{
		pContext->SetPipelineState(shaders.pLightingShadowEffect);
	}
	else if (bSpecular)
	{
		pContext->SetPipelineState(shaders.pLightingEffect);
	}
	else
	{
		pContext->SetPipelineState(shaders.pLightingNoSpecEffect);
	}
	pContext->SetDescriptorSet(mesh.pShadowDescriptorSet, 2);

	DrawMesh(pContext, mesh);
}

void DeferredShading::DrawLightVolumeFarPlane(GFXContext* pContext, const MeshData& mesh, const VolumeShader& shaders, bool bSpecular, bool bShadow)
{
	// Single pass only, draw if it passes the depth test and it's not an area tagged as sky

	if (bShadow)
	{
		pContext->SetPipelineState(shaders.pLightingFarPlaneShadowEffect);
		pContext->SetDescriptorSet(mesh.pShadowDescriptorSet, 2);
	}
	else if (bSpecular)
	{
		pContext->SetPipelineState(shaders.pLightingFarPlaneEffect);
	}
	else
	{
		pContext->SetPipelineState(shaders.pLightingFarPlaneNoSpecEffect);
	}

	pContext->SetDescriptorSet(mesh.pShadowDescriptorSet, 2);

	DrawMesh(pContext, mesh);
}


void DeferredShading::DrawMesh(GFXContext* pContext, const MeshData& mesh)
{
	pContext->SetVertexBuffer(mesh.pVB);
	pContext->DrawIndexed(mesh.pIB);
}


static inline void sincosf( float angle, float* psin, float* pcos )
{
    *psin = sinf( angle );
    *pcos = cosf( angle );
}

void DeferredShading::MakeSphere(GFXDevice* pDevice)
{
	const float fRadius = 1.0f;	// Will be scaled in the vertexShader;
	const uint32 uSlices = 16;
	const uint32 uStacks = 16;

	uint32 uIndices		= (2 * ( uStacks - 1 ) * uSlices)*3;
	uint32 uVertices	= ( uStacks - 1 ) * uSlices + 2;

	PositionVertex* pVertices = NULL;
	ScratchObj<PositionVertex> scratchVertices(pVertices, uVertices, 4);
	uint16* puIndices = NULL;
	ScratchObj<uint16> scratchIndices(puIndices, uIndices, 4);

    float fSinI[uSlices];
	float fCosI[uSlices];
    float fSinJ[uStacks];
	float fCosJ[uStacks];

    for(uint32 i = 0; i < uSlices; i++)
	{
		sincosf(2.0f * Math::pi * i / uSlices, fSinI + i, fCosI + i);
	}

    for(uint32 j = 0; j < uStacks; j++)
	{
        sincosf(Math::pi * j / uStacks, fSinJ + j, fCosJ + j);
	}

    // Generate vertices
    PositionVertex* pVertex = pVertices;

    // +Z pole
    pVertex->x = 0.0f;
	pVertex->y = 0.0f;
	pVertex->z = fRadius;
    pVertex++;

    // Stacks
    for(uint32 j = 1; j < uStacks; j++)
    {
        for(uint32 i = 0; i < uSlices; i++)
        {
            Vector3f norm(fSinI[i]* fSinJ[j], fCosI[i]* fSinJ[j], fCosJ[j]);
			norm.Normalise();	// Shouldn't be necessary, but ensure accuracy
			Vector3f pos = norm*fRadius;

			pVertex->x = pos.x;
			pVertex->y = pos.y;
			pVertex->z = pos.z;

            pVertex++;
        }
    }

    // Z- pole
    pVertex->x = 0.0f;
	pVertex->y =0.0f;
	pVertex->z = -fRadius;
    pVertex++;

    // Generate indices
    uint16* puFace = puIndices;
    uint16 uRowA, uRowB;

    // Z+ pole
    uRowA = 0;
    uRowB = 1;

	for (uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[2] = (uint16)(uRowA);
		puFace[1] = (uint16)(uRowB + i + 1);
		puFace[0] = (uint16)(uRowB + i);
		puFace += 3;
	}

	puFace[2] = (uint16)(uRowA);
	puFace[1] = (uint16)(uRowB);
	puFace[0] = (uint16)(uRowB + (uSlices - 1));
	puFace += 3;

	// Interior stacks
	for (uint32 j = 1; j < uStacks - 1; j++)
	{
		uRowA = 1 + (j - 1) * uSlices;
		uRowB = uRowA + uSlices;

		for (uint32 i = 0; i < uSlices - 1; i++)
		{
			puFace[2] = (uint16)(uRowA + i);
			puFace[1] = (uint16)(uRowA + i + 1);
			puFace[0] = (uint16)(uRowB + i);
			puFace += 3;

			puFace[2] = (uint16)(uRowA + i + 1);
			puFace[1] = (uint16)(uRowB + i + 1);
			puFace[0] = (uint16)(uRowB + i);
			puFace += 3;
		}

		puFace[2] = (uint16)(uRowA + uSlices - 1);
		puFace[1] = (uint16)(uRowA);
		puFace[0] = (uint16)(uRowB + uSlices - 1);
		puFace += 3;

		puFace[2] = (uint16)(uRowA);
		puFace[1] = (uint16)(uRowB);
		puFace[0] = (uint16)(uRowB + uSlices - 1);
		puFace += 3;
	}

	// Z- pole
	uRowA = 1 + (uStacks - 2) * uSlices;
	uRowB = uRowA + uSlices;

	for (uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[2] = (uint16)(uRowA + i);
		puFace[1] = (uint16)(uRowA + i + 1);
		puFace[0] = (uint16)(uRowB);
		puFace += 3;
	}

	puFace[2] = (uint16)(uRowA + uSlices - 1);
	puFace[1] = (uint16)(uRowA);
	puFace[0] = (uint16)(uRowB);

	m_sphereVB.Init(pDevice, pVertices, sizeof(PositionVertex), uVertices, "DeferredSphere");
	m_sphereIB.Init(pDevice, puIndices, uIndices, PT_TRIANGLES);
}


void DeferredShading::MakeCone(GFXDevice* pDevice)
{
	const float fRadius = 1.0f;	// Will be scaled in the vertexShader;
	const uint32 uSlices = 32;

	uint32 uBaseTriangles = uSlices;
	uint32 uConeTriangles = uSlices;

	uint32 uVertices	= uSlices + 2;	// Extras are the centre of the cone and source point
	uint32 uIndices		= (uBaseTriangles + uConeTriangles) * 3;

	PositionVertex* pVertices = NULL;
	ScratchObj<PositionVertex> scratchVertices(pVertices, uVertices, 4);
	uint16* puIndices = NULL;
	ScratchObj<uint16> scratchIndices(puIndices, uIndices, 4);

    float fSinI[uSlices];
	float fCosI[uSlices];

    for(uint32 i = 0; i < uSlices; i++)
	{
		sincosf(2.0f * Math::pi * i / uSlices, fSinI + i, fCosI + i);
	}


    // Generate vertices
    PositionVertex* pVertex = pVertices;

    // Cone top
    pVertex->x = 0.0f;
	pVertex->y = 0.0f;
	pVertex->z = 0.0f;
    pVertex++;

    // Base center
    pVertex->x = 0.0f;
    pVertex->y = 0.0f;
    pVertex->z = 1.0f;
    pVertex++;

    for(uint32 i = 0; i < uSlices; i++)
	{
		Vector3f norm(fSinI[i], fCosI[i], 0.0f);
		norm.Normalise();	// Shouldn't be necessary, but ensure accuracy
		Vector3f pos = norm*fRadius;

		pVertex->x = pos.x;
		pVertex->y = pos.y;
		pVertex->z = 1.0f;	// Put the z at unit distance

		pVertex++;
    }


    // Generate indices
    uint16* puFace = puIndices;

    
    uint16 uStart = 2;
   
    // The cone triangles
	for (uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[0] = i + uStart;
		puFace[1] = 0;
		puFace[2] = i + uStart + 1;
		puFace += 3;
	}

	puFace[0] = uSlices+uStart-1;
	puFace[1] = 0;
	puFace[2] = uStart;
	puFace += 3;
	
	
    // The base triangles
	for (uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[2] = i + uStart;
		puFace[1] = 1;
		puFace[0] = i + uStart + 1;
		puFace += 3;
	}

	puFace[2] = uSlices+uStart-1;
	puFace[1] = 1;
	puFace[0] = uStart;
	puFace += 3;

	m_coneVB.Init(pDevice, pVertices, sizeof(PositionVertex), uVertices, "DeferredCone");
	m_coneIB.Init(pDevice, puIndices, uIndices, PT_TRIANGLES);
}


void DeferredShading::MakeFrustum(GFXDevice* pDevice)
{
	const uint32 uIndicies = 36;
	uint16 iIndices[uIndicies] = 
	{
		0, 1, 3, 1, 2, 3,    // Top
		0, 3, 4, 3, 7, 4,    // Left
		3, 2, 7, 2, 6, 7,    // Front
		2, 1, 6, 1, 5, 6,    // Right
		1, 0, 5, 0, 4, 5,    // Back
		7, 6, 4, 6, 5, 4     // Bottom
	};

	PositionVertex verts[8] =
	{
		// Top
		{ -1.0f,  1.0f, -1.0f }, // 0 - BL
		{  1.0f,  1.0f, -1.0f }, // 1 - BR
		{  1.0f,  1.0f,  1.0f }, // 2 - FR
		{ -1.0f,  1.0f,  1.0f }, // 3 - FL
		// Bottom
		{ -1.0f, -1.0f, -1.0f }, // 4 - BL
		{  1.0f, -1.0f, -1.0f }, // 5 - BR
		{  1.0f, -1.0f,  1.0f }, // 6 - FR
		{ -1.0f, -1.0f,  1.0f }, // 7 - FL
	};


	m_frustumMesh.Init(pDevice, verts, sizeof(PositionVertex), 8, "ProjLight", GPU_USAGE_STATIC);
	m_frustumIB.Init(pDevice, iIndices, uIndicies, PT_TRIANGLES);
}

}


#endif
